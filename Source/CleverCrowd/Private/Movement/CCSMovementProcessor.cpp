// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/CCSMovementProcessor.h"

#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Management/CCSEntitiesManagerSubsystem.h"
#include "Movement/MovementFragments.h"

UCCSMovementProcessor::UCCSMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UCCSMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCCSMovementProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	EntitiesManagerSubsystem = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>();
}

void UCCSMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaSeconds = FMath::Min(GetWorld()->GetDeltaSeconds(), 0.1f);
	const TArray<float>& SpeedInClusters = EntitiesManagerSubsystem->MovementSpeedsInClusters;
	const TArray<float>& SpeedInAreas = EntitiesManagerSubsystem->MovementSpeedsInAreas;
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &DeltaSeconds, &SpeedInClusters, &SpeedInAreas](FMassExecutionContext& Context)
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
			const FVector PrevEntityLocation      = TransformFragment.GetMutableTransform().GetLocation();

			float EntityMovementSpeed = SpeedInClusters[ClusterFragment.ClusterType];
			if (ClusterFragment.AreaId > INDEX_NONE)
			{
				EntityMovementSpeed = SpeedInAreas[ClusterFragment.AreaId];
			}
			
			const FVector DeltaLocation     = ForceFragment.Value * DeltaSeconds * EntityMovementSpeed;
			const FVector NewEntityLocation = PrevEntityLocation + DeltaLocation;
			TransformFragment.GetMutableTransform().SetLocation(NewEntityLocation);

			if (MovementFragment.PrevTickLocation == FVector::ZeroVector)
			{
				MovementFragment.PrevTickLocation = NewEntityLocation;
			}
			MovementFragment.CurrentSpeed = FVector::Distance(NewEntityLocation, PrevEntityLocation) / DeltaSeconds;
		}
	});
}
