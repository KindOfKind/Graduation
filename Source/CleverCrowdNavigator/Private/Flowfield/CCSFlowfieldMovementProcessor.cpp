// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/CCSFlowfieldMovementProcessor.h"

#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "Flowfield/Flowfield.h"
#include "Flowfield/GoalPoint.h"
#include "Global/CrowdNavigationSubsystem.h"
#include "Movement/MovementFragments.h"

UCCSFlowfieldMovementProcessor::UCCSFlowfieldMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	bRequiresGameThreadExecution = true;

	CrowdNavigationSubsystem = nullptr;
}

void UCCSFlowfieldMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FNavigationFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCCSFlowfieldMovementProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	CrowdNavigationSubsystem = GetWorld()->GetSubsystem<UCrowdNavigationSubsystem>();
}

void UCCSFlowfieldMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaSeconds = FMath::Min(GetWorld()->GetDeltaSeconds(), 0.1f);
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &DeltaSeconds, &EntityManager](FMassExecutionContext& Context)
	{
		const int32 NumEntities                              = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList   = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassForceFragment> ForceList       = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FNavigationFragment> NavigationList = Context.GetMutableFragmentView<FNavigationFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment   = TransformList[EntityIndex];
			FMassForceFragment& ForceFragment       = ForceList[EntityIndex];
			FNavigationFragment& NavigationFragment = NavigationList[EntityIndex];
			const FVector EntityLocation            = TransformFragment.GetMutableTransform().GetLocation();

			const int32 DirectionsGridIndex = NavigationFragment.GoalPointIndex;
			const bool bUseDetour           = NavigationFragment.bCanUseDetour && bDetourAvailable;
			const FVector Direction         = CrowdNavigationSubsystem->GetFlowfield()->GetDirectionAtLocation(EntityLocation, DirectionsGridIndex, bUseDetour);
			ForceFragment.Value             = Direction;

			AGoalPoint* GoalPoint = CrowdNavigationSubsystem->GetFlowfield()->GetGoalPoint(DirectionsGridIndex);
			auto IsInInteractionArea = [GoalPoint, &EntityLocation]() -> bool
			{
				return FVector::Dist2D(GoalPoint->GetActorLocation(), EntityLocation) <= GoalPoint->EntityInteractionRange;
			};
			if (IsValid(GoalPoint) && IsInInteractionArea())
			{
				// @warning: Be aware that entity may be destroyed after interaction with ExitPoint
				GoalPoint->InteractWithEntity(EntityManager, Context.GetEntity(EntityIndex));
			}
		}
	});
}
