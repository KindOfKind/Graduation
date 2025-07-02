// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowd/CrowdMetricsProcessor.h"

#include "CrowdEvaluationHashGrid.h"
#include "GameEvaluatorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Collisions/CollisionsFragments.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Flowfield/Flowfield.h"
#include "Global/CrowdNavigationSubsystem.h"
#include "Grids/DebugBPFunctionLibrary.h"
#include "Management/CCSEntitiesManagerSubsystem.h"
#include "Misc/TransactionObjectEvent.h"
#include "Movement/MovementFragments.h"


UCrowdMetricsProcessor::UCrowdMetricsProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowdMetricsProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FNavigationFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCrowdMetricsProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	
	EntitiesHashGrid         = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>()->GetEntitiesHashGrid();
	CrowdNavigationSubsystem = GetWorld()->GetSubsystem<UCrowdNavigationSubsystem>();
}

void UCrowdMetricsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	if (!IsValid(GameEvaluator))
	{
		GameEvaluator = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	}
	if (GameEvaluator->GetAgentsEvaluationResultMutable().CrowdGroups.IsEmpty())
	{
		InitDefaultCrowdGroup();
	}
	ACrowdEvaluationHashGrid* EvaluationGrid = GameEvaluator->GetEvaluationHashGrid();
	EvaluationGrid->SnapshotsRate = CrowdRegroupRate * MetricsSnapshotRate;
	
	bool bDoSnapshot = true;
	bool bShouldRegroup = false;
	
	UWorld* World = EntityManager.GetWorld();
	CurrentTime   = World->GetTimeSeconds();
	if (CurrentTime < NextSnapshotTime)
	{
		bDoSnapshot = false;
	}
	if (bDoSnapshot)
	{
		NextSnapshotTime += MetricsSnapshotRate;
		SnapshotsBeforeRegroup -= 1;
		bShouldRegroup = (SnapshotsBeforeRegroup <= 0);
		if (bShouldRegroup)
		{
			SnapshotsBeforeRegroup = CrowdRegroupRate;
		}
	}

	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager, bDoSnapshot, bShouldRegroup, EvaluationGrid](FMassExecutionContext& Context)
	{
		const int32 NumEntities                              = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList   = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FClusterFragment> ClusterList       = Context.GetMutableFragmentView<FClusterFragment>();
		const TArrayView<FCollisionFragment> CollisionList   = Context.GetMutableFragmentView<FCollisionFragment>();
		const TArrayView<FNavigationFragment> NavigationList = Context.GetMutableFragmentView<FNavigationFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment   = TransformList[EntityIndex];
			FClusterFragment& ClusterFragment       = ClusterList[EntityIndex];
			FCollisionFragment& CollisionFragment   = CollisionList[EntityIndex];
			FNavigationFragment& NavigationFragment = NavigationList[EntityIndex];
			const FVector EntityLocation            = TransformFragment.GetMutableTransform().GetLocation();

			constexpr bool bDebugDelete = false;
			if (bDebugDelete)
			{
				if (Context.GetEntity(EntityIndex).SerialNumber < 2000)
				{
					EntityManager.Defer().DestroyEntity(Context.GetEntity(EntityIndex));
					continue;
				}
			}

			FEntityData EntityData
			{
				Context.GetEntity(EntityIndex),
				TransformFragment.GetMutableTransform(),
				ClusterFragment,
				CollisionFragment,
				NavigationFragment
			};

			if (bDebugCrowdGroup)
			{
				DebugDrawCrowdGroupOfEntity(EntityManager, EntityData);
			}

			if (!bDoSnapshot)
			{
				continue;
			}
			MakeSnapshot(EntityManager, EntityData);

			if (bShouldRegroup)
			{
				const int32 GroupIndex = ReassignCrowdGroupToEntity(EntityManager, EntityData);
				if (GroupIndex > 0)
				{
					EvaluationGrid->AddAgentToCrowdGroupCell(EntityData.Transform.GetLocation(), GroupIndex);
				}
			}
		}
	});

	if (bShouldRegroup)
	{
		EvaluationGrid->MakeCrowdGroupAreasSnapshot();
		EvaluationGrid->ClearCrowdGroupCells();
		EvaluationGrid->DebugDrawCrowdGroupAreas(GetWorld(), 6.f, 8.f);
	}
}

void UCrowdMetricsProcessor::MakeSnapshot(FMassEntityManager& EntityManager, const FEntityData& EntityData)
{
	// Constructs metrics snapshot and caches it into the Evaluator subsystem
	FCrowdAgentMetricsSnapshot Snapshot;
	Snapshot.Metrics.Location          = EntityData.Transform.GetLocation();
	Snapshot.Metrics.MovementDirection = CrowdNavigationSubsystem->GetFlowfield()->GetDirectionAtLocation(
		EntityData.Transform.GetLocation(), EntityData.NavigationFragment.GoalPointIndex, EntityData.NavigationFragment.bCanUseDetour);
	Snapshot.Metrics.StrongCollisions = EntityData.CollisionFragment.StrongCollisionsCounterMeta;
	Snapshot.Metrics.WeakCollisions   = EntityData.CollisionFragment.WeakCollisionsCounterMeta;

	// TestMaxWeakCollisionsCached = FMath::Max(TestMaxWeakCollisionsCached, Snapshot.Metrics.WeakCollisions);
	// TestMaxStrongCollisionsCached = FMath::Max(TestMaxStrongCollisionsCached, Snapshot.Metrics.StrongCollisions);
	
	GameEvaluator->GetAgentsEvaluationResultMutable().AddAgentMetricsSnapshot(EntityData.Entity, Snapshot);

	EntityData.CollisionFragment.StrongCollisionsCounterMeta = 0;
}

int32 UCrowdMetricsProcessor::ReassignCrowdGroupToEntity(FMassEntityManager& EntityManager, const FEntityData& EntityData)
{
	constexpr float SimilarityThreshold = 0.8f;	// If similarity is less than this threshold, a new CrowdGroup will be created
	constexpr float ChangeGroupCost = 0.05f;
	constexpr bool bEvaluateDirectionAtBiggerRange = false;
	
	const float SnapshotsTimeDiff = CrowdRegroupRate * MetricsSnapshotRate;

	FCrowdAgentsEvaluationResult& Evaluation       = GameEvaluator->GetAgentsEvaluationResultMutable();
	const int32 LastSnapshotIdx                    = Evaluation.GetSnapshotIndexAtTimeSafe(CurrentTime, MetricsSnapshotRate);
	const int32 EarlySnapshotIdx                   = Evaluation.GetSnapshotIndexAtTimeSafe(CurrentTime - SnapshotsTimeDiff, MetricsSnapshotRate);
	const int32 TwiceEarlySnapshotIdx              = Evaluation.GetSnapshotIndexAtTimeSafe(CurrentTime - SnapshotsTimeDiff - 7.f, MetricsSnapshotRate);	// Temp
	FCrowdAgentMetricsSnapshot& LastSnapshot       = Evaluation.GetSnapshot(EntityData.Entity, LastSnapshotIdx);
	FCrowdAgentMetricsSnapshot& EarlySnapshot      = Evaluation.GetSnapshot(EntityData.Entity, EarlySnapshotIdx);
	FCrowdAgentMetricsSnapshot& TwiceEarlySnapshot = Evaluation.GetSnapshot(EntityData.Entity, TwiceEarlySnapshotIdx);	// Temp

	// Make aggregated metrics by iterating through 10 snapshots (to analyze dynamic of an agent)
	FCrowdAgentMetricsMag AccumulatedMetrics;
	for (float SnapshotIdx = EarlySnapshotIdx; SnapshotIdx < LastSnapshotIdx - 1; SnapshotIdx++)
	{
		FCrowdAgentMetricsSnapshot DiffSnapshot = FCrowdAgentMetricsSnapshot::GetSnapshotsDelta(
			Evaluation.GetSnapshot(EntityData.Entity, SnapshotIdx), Evaluation.GetSnapshot(EntityData.Entity, SnapshotIdx + 1));
		AccumulatedMetrics.PathWalked += DiffSnapshot.Metrics.Location.Size();
		AccumulatedMetrics.MovementDirection += DiffSnapshot.Metrics.MovementDirection.Rotation().Yaw;
		AccumulatedMetrics.WeakCollisions += DiffSnapshot.Metrics.WeakCollisions;
		AccumulatedMetrics.StrongCollisions += DiffSnapshot.Metrics.StrongCollisions;
	}
	float AccumulatedMovementDirection = 0.f;	// Temp solution. We want to evaluate movement direction changes at a bigger time range
	int32 MovementEarlySnapshotIdx = (bEvaluateDirectionAtBiggerRange ? TwiceEarlySnapshotIdx : EarlySnapshotIdx);
	for (float SnapshotIdx = MovementEarlySnapshotIdx; SnapshotIdx < LastSnapshotIdx - 1; SnapshotIdx++)
	{
		FCrowdAgentMetricsSnapshot DiffSnapshot = FCrowdAgentMetricsSnapshot::GetSnapshotsDelta(
			Evaluation.GetSnapshot(EntityData.Entity, SnapshotIdx), Evaluation.GetSnapshot(EntityData.Entity, SnapshotIdx + 1));
		AccumulatedMovementDirection += DiffSnapshot.Metrics.MovementDirection.Rotation().Yaw;
	}

	FCrowdAgentMetricsMag DeltaMetrics = FCrowdAgentMetricsSnapshot::GetSnapshotsDeltaNormalized(
		EarlySnapshot, LastSnapshot, AccumulatedMetrics, SnapshotsTimeDiff);
	if (bEvaluateDirectionAtBiggerRange)
	{
		DeltaMetrics.MovementDirection = FMath::Clamp(AccumulatedMovementDirection / (FCrowdAgentMetricsSnapshot::MaxRotation * SnapshotsTimeDiff + 10.f), 0.0, 1.0);	// Temp
	}

	// Search for the most similar CrowdGroup
	const int32 GroupsNum             = Evaluation.CrowdGroups.Num();
	float MaxSimilarityWithoutPenalty = 0.f;
	float MaxSimilarity               = 0.f;
	int32 MaxSimilarityGroupIdx       = -1;
	for (int32 GroupIdx = 0; GroupIdx < GroupsNum; GroupIdx++)
	{
		float Similarity            = CalculateAgentsSimilarityCoef(DeltaMetrics, Evaluation.CrowdGroups[GroupIdx].AverageAgentMetrics);
		MaxSimilarityWithoutPenalty = FMath::Max(MaxSimilarityWithoutPenalty, Similarity);
		if (LastSnapshot.CrowdGroupIndex != GroupIdx)
		{
			Similarity -= ChangeGroupCost;
		}
		if (MaxSimilarity < Similarity)
		{
			MaxSimilarity = Similarity;
			MaxSimilarityGroupIdx = GroupIdx;
		}
	}

	if (MaxSimilarity <= 0)
	{
		if (Evaluation.CrowdGroups.IsEmpty())
		{
			// ToDo: refactor duplicated code
			FCrowdGroup NewCrowdGroup;
			NewCrowdGroup.AverageAgentMetrics = DeltaMetrics;
			Evaluation.CrowdGroups.Add(NewCrowdGroup);
			LastSnapshot.CrowdGroupIndex = Evaluation.CrowdGroups.Num() - 1;
		}
		return LastSnapshot.CrowdGroupIndex;
	}
	// Either create a new group or assign to the most alike
	if (MaxSimilarityWithoutPenalty < SimilarityThreshold)
	{
		FCrowdGroup NewCrowdGroup;
		NewCrowdGroup.AverageAgentMetrics = DeltaMetrics;
		Evaluation.CrowdGroups.Add(NewCrowdGroup);
		LastSnapshot.CrowdGroupIndex = Evaluation.CrowdGroups.Num() - 1;
	}
	else if (LastSnapshot.CrowdGroupIndex != MaxSimilarityGroupIdx &&
			MaxSimilarity - ChangeGroupCost >= SimilarityThreshold)
	{
		LastSnapshot.CrowdGroupIndex = MaxSimilarityGroupIdx;
	}

	return LastSnapshot.CrowdGroupIndex;
}

float UCrowdMetricsProcessor::CalculateAgentsSimilarityCoef(const FCrowdAgentMetricsMag& Metrics, const FCrowdAgentMetricsMag& OtherMetrics)
{
	float Similarity = 0.0f;
	TArray<int32> DebugMetricsToSkip = {};

	const int32 MetricsNum = FCrowdAgentMetricsMag::FloatMetricsNum;
	float SimilaritiesSum = 0.f;
	for (int32 MetricIdx = 0; MetricIdx < MetricsNum; MetricIdx++)
	{
		if (DebugMetricsToSkip.Contains(MetricIdx)) continue;
		SimilaritiesSum += 1 - abs(Metrics.GetFloatMetric(MetricIdx) - OtherMetrics.GetFloatMetric(MetricIdx));
	}
	Similarity = SimilaritiesSum / (MetricsNum - DebugMetricsToSkip.Num());	// Average of similarities of all metrics

	// Cosine similarity (to compare vectors directions)
	// float MulSum = 0.f; float SumA = 0.f; float SumB = 0.f;
	// for (int32 MetricIdx = 0; MetricIdx < MetricsNum; MetricIdx++)
	// {
	// 	MulSum += Metrics.GetFloatMetric(MetricIdx) * OtherMetrics.GetFloatMetric(MetricIdx);
	// 	SumA += pow(Metrics.GetFloatMetric(MetricIdx), 2);
	// 	SumB += pow(OtherMetrics.GetFloatMetric(MetricIdx), 2);
	// }
	// Similarity = MulSum / (sqrt(SumA) * sqrt(SumB));

	return Similarity;
}

void UCrowdMetricsProcessor::InitDefaultCrowdGroup()
{
	FCrowdGroup NewCrowdGroup;
	NewCrowdGroup.AverageAgentMetrics.Location          = 0.3f;
	NewCrowdGroup.AverageAgentMetrics.PathWalked        = 0.3f;
	NewCrowdGroup.AverageAgentMetrics.MovementDirection = 0.f;
	NewCrowdGroup.AverageAgentMetrics.WeakCollisions    = 0.f;
	NewCrowdGroup.AverageAgentMetrics.StrongCollisions  = 0.f;
	
	FCrowdAgentsEvaluationResult& Evaluation = GameEvaluator->GetAgentsEvaluationResultMutable();
	Evaluation.CrowdGroups.Add(NewCrowdGroup);
}

void UCrowdMetricsProcessor::DebugDrawCrowdGroupOfEntity(FMassEntityManager& EntityManager, const FEntityData& EntityData)
{
	if (!GameEvaluator->GetAgentsEvaluationResultMutable().IsValidSnapshotIndex(EntityData.Entity, 0))
	{
		return;
	}
	const int32 CrowdGroupIndex = GameEvaluator->GetAgentsEvaluationResultMutable().GetLastSnapshot(EntityData.Entity).CrowdGroupIndex;
	// if (CrowdGroupIndex == 0)
	// {
	// 	return;
	// }
	FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(CrowdGroupIndex);
	const FVector Location = EntityData.Transform.GetLocation();
	DrawDebugLine(EntityManager.GetWorld(), Location, Location + FVector::UpVector * 200.f, DebugColor, false, -1.f, 0, 7.f);
}
