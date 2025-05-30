// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/CommonStatisticsProcessor.h"

#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "Common/CommonTypes.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Movement/MovementFragments.h"

UCommonStatisticsProcessor::UCommonStatisticsProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCommonStatisticsProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCommonStatisticsProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	CrowdStatistics = GetWorld()->GetSubsystem<UCrowdStatisticsSubsystem>();
}

void UCommonStatisticsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaSeconds = FMath::Min(GetWorld()->GetDeltaSeconds(), 0.1f);

	TArray<int32> EntitiesInAreas;
	EntitiesInAreas.AddDefaulted(FCrowdStatistics::MaxClusterType);
	TArray<int32> EntitiesInClusters;
	EntitiesInClusters.AddDefaulted(FCrowdStatistics::MaxClusterType);

	TArray<FAggregatedValueFloat> AggregatedSpeedInAreas;
	AggregatedSpeedInAreas.AddDefaulted(FCrowdStatistics::MaxClusterType);
	TArray<FAggregatedValueFloat> AggregatedSpeedInClusters;
	AggregatedSpeedInClusters.AddDefaulted(FCrowdStatistics::MaxClusterType);
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&, this](FMassExecutionContext& Context)
	{
		const int32 NumEntities                            = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassForceFragment> ForceList     = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FMovementFragment> MovementList   = Context.GetMutableFragmentView<FMovementFragment>();
		const TArrayView<FClusterFragment> ClusterList     = Context.GetMutableFragmentView<FClusterFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FMassForceFragment& ForceFragment     = ForceList[EntityIndex];
			FMovementFragment& MovementFragment   = MovementList[EntityIndex];
			FClusterFragment& ClusterFragment     = ClusterList[EntityIndex];
			const FVector EntityLocation          = TransformFragment.GetMutableTransform().GetLocation();
			const int32 ClusterType               = ClusterFragment.ClusterType;

			EntitiesInClusters[ClusterType] += 1;
			AggregatedSpeedInClusters[ClusterType].AddValue(MovementFragment.CurrentSpeed);
			
			if (ClusterFragment.AreaId > INDEX_NONE)
			{
				int32 AreasToAdd = ClusterFragment.AreaId - EntitiesInAreas.Num() + 1;
				for (int32 i = 0; i < AreasToAdd; i++)
				{
					EntitiesInAreas.AddDefaulted();
					AggregatedSpeedInAreas.AddDefaulted();
				}
				EntitiesInAreas[ClusterFragment.AreaId] += 1;
				AggregatedSpeedInAreas[ClusterFragment.AreaId].AddValue(MovementFragment.CurrentSpeed);
			}
		}
	});

	CrowdStatistics->Stats.UpdateAgentsCountersInClusters(EntitiesInClusters);
	CrowdStatistics->Stats.UpdateAgentsCountersInAreas(EntitiesInAreas);
	for (int32 ClusterIndex = 0; ClusterIndex < FCrowdStatistics::MaxClusterType; ClusterIndex++)
	{
		CrowdStatistics->Stats.AverageEntitySpeedInClusters[ClusterIndex] = AggregatedSpeedInClusters[ClusterIndex].GetMean();
	}
	
	int32 AreaTypesNum = EntitiesInAreas.Num();
	for (int32 AreaType = 0; AreaType < AreaTypesNum; AreaType++)
	{
		CrowdStatistics->Stats.AverageEntitySpeedInAreas[AreaType] = AggregatedSpeedInAreas[AreaType].GetMean();
	}
}
