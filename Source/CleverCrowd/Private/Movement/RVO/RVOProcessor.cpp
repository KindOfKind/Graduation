// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/RVO/RVOProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationUtils.h"
#include "OrcaSolver.h"
#include "Collisions/CollisionsFragments.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "HashGrid/CCSEntitiesHashGrid.h"
#include "Management/CCSCollisionsSubsystem.h"
#include "Management/CCSEntitiesManagerSubsystem.h"

URVOProcessor::URVOProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Avoidance;
	bRequiresGameThreadExecution = true;
}

void URVOProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void URVOProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	EntitiesHashGrid         = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>()->GetEntitiesHashGrid();
	CrowdStatisticsSubsystem = GetWorld()->GetSubsystem<UCrowdStatisticsSubsystem>();
	EntitiesManagerSubsystem = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>();
	CollisionsSubsystem      = GetWorld()->GetSubsystem<UCCSCollisionsSubsystem>();
}

void URVOProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	constexpr int ClusterSize = 2;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTime = GetWorld()->DeltaTimeSeconds;

	const bool bUseToTheSideAvoidance = (EntitiesManagerSubsystem->AvoidanceType == 0);
	const bool bUseSimpleAvoidance    = (EntitiesManagerSubsystem->AvoidanceType == 1);
	const bool bUseORCA               = (EntitiesManagerSubsystem->AvoidanceType == 2);
	
	if (bUseORCA && !IsValid(OrcaSolver))
	{
		InitializeOrcaSolver();
	}

	// Update ORCA data and do To-The-Side-Avoidance
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&, this](FMassExecutionContext& Context)
	{
		const int32 NumEntities                            = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FCollisionFragment> CollisionList = Context.GetMutableFragmentView<FCollisionFragment>();
		const TArrayView<FMassForceFragment> ForceList     = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FClusterFragment> ClusterList     = Context.GetMutableFragmentView<FClusterFragment>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FCollisionFragment& CollisionFragment = CollisionList[EntityIndex];
			FMassForceFragment& ForceFragment     = ForceList[EntityIndex];
			FClusterFragment& ClusterFragment     = ClusterList[EntityIndex];

			if (bUseORCA)
			{
				if (!bInitializedOrcaAgents)
				{
					CollisionFragment.OrcaIndex = OrcaSolver->AddAgent(TransformFragment.GetTransform().GetLocation());
				}
				else
				{
					OrcaSolver->SetAgentLocation(CollisionFragment.OrcaIndex, TransformFragment.GetTransform().GetLocation());
				}
				OrcaSolver->SetPreferredVelocity(CollisionFragment.OrcaIndex, ForceFragment.Value.GetSafeNormal());
			}

			if (bUseToTheSideAvoidance)
			{
				float ToTheSideAvoidanceDuration = EntitiesManagerSubsystem->DefaultToTheSideAvoidanceDuration;
				if (ClusterFragment.AreaId > INDEX_NONE)
				{
					ToTheSideAvoidanceDuration = EntitiesManagerSubsystem->ToTheSideAvoidanceDurationInAreas[ClusterFragment.AreaId];
				}

				if (ToTheSideAvoidanceDuration >= 0.05f)
				{
					DoToTheSideAvoidance(CollisionFragment, ForceFragment.Value, TransformFragment.GetMutableTransform(), Context, ToTheSideAvoidanceDuration, DeltaTime);
				}
			}
		}
	});

	// Update agents locations with Orca solver
	if (bUseORCA)
	{
		OrcaSolver->SetTimeStep(DeltaTime);
		OrcaSolver->DoStep();
		
		EntityQuery.ForEachEntityChunk(EntityManager, Context, [&, this](FMassExecutionContext& Context)
		{
			const int32 NumEntities                            = Context.GetNumEntities();
			const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
			const TArrayView<FCollisionFragment> CollisionList = Context.GetMutableFragmentView<FCollisionFragment>();

			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				FTransformFragment& TransformFragment = TransformList[EntityIndex];
				FCollisionFragment& CollisionFragment = CollisionList[EntityIndex];

				const FVector& OldLocation = TransformFragment.GetMutableTransform().GetLocation();
				FVector NewLocation        = OrcaSolver->GetAgentLocation(CollisionFragment.OrcaIndex);
				TransformFragment.GetMutableTransform().SetLocation(FVector{NewLocation.X, NewLocation.Y, OldLocation.Z});
			}
		});
	}

	bInitializedOrcaAgents = true;

	// Simple custom avoidance
	if (bUseSimpleAvoidance)
	{
		EntitiesHashGrid->ParallelForEachNonEmptyCell([&, this](const FGridCellPosition& Cell)
		{
			TArray<FMassEntityHandle> Entities;
			FGridBounds Bounds{Cell, Cell + FGridCellPosition{ClusterSize - 1, ClusterSize - 1}};
			EntitiesHashGrid->GetEntitiesInBounds(Entities, Bounds); // Getting entities from 4 adjacent grid cells

			const int32 EntitiesNum = Entities.Num();
			for (int32 EntityIndex = 0; EntityIndex < EntitiesNum; ++EntityIndex)
			{
				FTransform& Transform                 = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[EntityIndex]).GetMutableTransform();
				const float Radius                    = EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entities[EntityIndex]).Radius;
				FVector& Force                        = EntityManager.GetFragmentDataChecked<FMassForceFragment>(Entities[EntityIndex]).Value;
				FCollisionFragment& CollisionFragment = EntityManager.GetFragmentDataChecked<FCollisionFragment>(Entities[EntityIndex]);
				FClusterFragment& ClusterFragment     = EntityManager.GetFragmentDataChecked<FClusterFragment>(Entities[EntityIndex]);
				const FVector& Location               = Transform.GetLocation();
				FVector Location2D                    = FVector{Location.X, Location.Y, 0.f};

				float AvoidanceRadius   = EntitiesManagerSubsystem->AvoidanceRadiusInClusters[ClusterFragment.ClusterType];
				float AvoidanceStrength = EntitiesManagerSubsystem->AvoidanceStrengthInClusters[ClusterFragment.ClusterType];
				if (ClusterFragment.AreaId > INDEX_NONE)
				{
					AvoidanceRadius   = EntitiesManagerSubsystem->AvoidanceRadiusInAreas[ClusterFragment.AreaId];
					AvoidanceStrength = EntitiesManagerSubsystem->AvoidanceStrengthInAreas[ClusterFragment.AreaId];
				}

				if (AvoidanceRadius <= 1.f || AvoidanceStrength <= 0.01f)
				{
					continue;
				}

				FEntityProxyData EntityData{
					Transform, Location2D, Radius, Force, CollisionFragment, ClusterFragment
				};

				for (int32 OtherEntityIndex = 0; OtherEntityIndex < EntitiesNum; ++OtherEntityIndex)
				{
					if (Entities[EntityIndex] == Entities[OtherEntityIndex])
					{
						continue;
					}

					FTransform& OtherTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[OtherEntityIndex]).GetMutableTransform();
					const float OtherRadius = EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entities[OtherEntityIndex]).Radius;
					FVector& OtherForce = EntityManager.GetFragmentDataChecked<FMassForceFragment>(Entities[OtherEntityIndex]).Value;
					FCollisionFragment& OtherCollisionFragment = EntityManager.GetFragmentDataChecked<FCollisionFragment>(Entities[OtherEntityIndex]);
					FClusterFragment& OtherClusterFragment = EntityManager.GetFragmentDataChecked<FClusterFragment>(Entities[OtherEntityIndex]);
					const FVector& OtherLocation = OtherTransform.GetLocation();
					FVector OtherLocation2D = FVector{OtherLocation.X, OtherLocation.Y, 0.f};

					FEntityProxyData OtherEntityData{
						OtherTransform, OtherLocation2D, OtherRadius, OtherForce, OtherCollisionFragment, OtherClusterFragment
					};

					DoSimpleAvoidance(EntityData, OtherEntityData, AvoidanceRadius, AvoidanceStrength, DeltaTime);
				}
			}
		});
	}
}

void URVOProcessor::DoSimpleAvoidance(FEntityProxyData& EntityData, FEntityProxyData& OtherEntityData, float AvoidanceRadius, float AvoidanceStrength, float DeltaTime)
{
	FVector RelativeLocation = OtherEntityData.Location - EntityData.Location;
	float Distance = RelativeLocation.Length();
	FVector NewForce = EntityData.Force;

	if (Distance == 0.f || Distance > AvoidanceRadius)
	{
		return;
	}
	FVector AvoidanceDirection = FRotator{0.f, 20.f, 0.f}.RotateVector(RelativeLocation.GetSafeNormal());
	NewForce -= AvoidanceDirection * AvoidanceStrength / Distance;
	EntityData.Force = NewForce;

	// Currently we increase the collisions counter when avoiding - to search for dense crowd areas more effectively.
	// Note that we don't add collision to statistics here, unlike we do in CollisionsProcessor.
	float CurrentTime = GetWorld()->GetTimeSeconds();
	constexpr float MinCollisionCountRateForEntity = 2.f; // @warning: this value is copied from CollisionsProcessor
	if (CurrentTime - EntityData.CollisionFragment.LastCollisionCountTime > MinCollisionCountRateForEntity)
	{
		EntityData.CollisionFragment.LastCollisionCountTime = CurrentTime;
		CollisionsSubsystem->GetCollisionsHashGrid().AddCollisionsCountAtLocation(EntityData.Location, 1);
	}
}

void URVOProcessor::DoToTheSideAvoidance(FCollisionFragment& CollisionFragment, FVector& Force, FTransform& Transform, FMassExecutionContext& Context,
                                         float Duration, float DeltaTime)
{
	constexpr float TurnDegree = 40.f;

	auto AvoidLambda = [this](FCollisionFragment& CollisionFragment, FVector& Force)
	{
		float DeltaYaw = TurnDegree * (CollisionFragment.bAvoidToTheRight ? 1.f : -1.f);
		Force = FRotator{ 0.f, -DeltaYaw, 0.f }.RotateVector(Force);
	};

	if (CollisionFragment.AvoidingToSideTimeLeft > 0.f)
	{
		CollisionFragment.AvoidingToSideTimeLeft -= DeltaTime;

		if (CollisionFragment.AvoidingToSideTimeLeft > 0.f)
		{
			// Continue avoiding
			AvoidLambda(CollisionFragment, Force);
			return;
		}
	}

	if (CollisionFragment.bCollidedAtPreviousTick == false)
	{
		return;
	}

	// Start avoiding to random side
	CollisionFragment.bAvoidToTheRight       = FMath::RandBool();
	CollisionFragment.AvoidingToSideTimeLeft = Duration;
	AvoidLambda(CollisionFragment, Force);
	//DrawDebugLine(Context.GetWorld(), Transform.GetLocation(), Transform.GetLocation() + FVector::UpVector * 1000.f, FColor::Yellow, false, 2.f, 0, 10.f);
}

void URVOProcessor::InitializeOrcaSolver()
{
	const FOrcaDefaultAgentParams AgentDefaults;
	
	OrcaSolver = NewObject<UOrcaSolver>();
	OrcaSolver->Initialize(AgentDefaults);
}


// ToDo: implement
void URVOProcessor::DoMassORCA(FEntityProxyData& EntityData, TArray<FEntityProxyData>& OtherEntityDatas, float AvoidanceRadius, float AvoidanceStrength, float DeltaTime)
{
	// const FVector DesAcc = UE::MassNavigation::ClampVector(SteeringForce, MaxSteerAccel);
	// const FVector DesVel = UE::MassNavigation::ClampVector(AgentVelocity + DesAcc * DeltaTime, MaximumSpeed);
	//
	// for (const FCollider& Collider : Colliders)
	// {
	// 	bool bHasForcedNormal = false;
	// 	FVector ForcedNormal  = FVector::ZeroVector;
	//
	// 	if (Collider.bCanAvoid == false)
	// 	{
	// 		// If the other obstacle cannot avoid us, try to avoid the local minima they create between the wall and their collider.
	// 		// If the space between edge and collider is less than MinClearance, make the agent to avoid the gap.
	// 		const FVector::FReal MinClearance = 2. * AgentRadius * MovingAvoidanceParams.StaticObstacleClearanceScale;
	//
	// 		// Find the maximum distance from edges that are too close.
	// 		FVector::FReal MaxDist = -1.;
	// 		FVector ClosestPoint   = FVector::ZeroVector;
	// 		for (const FNavigationAvoidanceEdge& Edge : NavEdges.AvoidanceEdges)
	// 		{
	// 			const FVector Point  = FMath::ClosestPointOnSegment(Collider.Location, Edge.Start, Edge.End);
	// 			const FVector Offset = Collider.Location - Point;
	// 			if (FVector::DotProduct(Offset, Edge.LeftDir) < 0.)
	// 			{
	// 				// Behind the edge, ignore.
	// 				continue;
	// 			}
	//
	// 			const FVector::FReal OffsetLength = Offset.Length();
	// 			const bool bTooNarrow             = (OffsetLength - Collider.Radius) < MinClearance;
	// 			if (bTooNarrow)
	// 			{
	// 				if (OffsetLength > MaxDist)
	// 				{
	// 					MaxDist      = OffsetLength;
	// 					ClosestPoint = Point;
	// 				}
	// 			}
	// 		}
	//
	// 		if (MaxDist != -1.)
	// 		{
	// 			// Set up forced normal to avoid the gap between collider and edge.
	// 			ForcedNormal     = (Collider.Location - ClosestPoint).GetSafeNormal();
	// 			bHasForcedNormal = true;
	// 		}
	// 	}
	//
	// 	FVector RelPos               = AgentLocation - Collider.Location;
	// 	RelPos.Z                     = 0.; // we assume we work on a flat plane for now
	// 	const FVector RelVel         = DesVel - Collider.Velocity;
	// 	const FVector::FReal ConDist = RelPos.Size();
	// 	const FVector ConNorm        = ConDist > 0. ? RelPos / ConDist : FVector::ForwardVector;
	//
	// 	FVector SeparationNormal = ConNorm;
	// 	if (bHasForcedNormal)
	// 	{
	// 		// The more head on the collisions is, the more we should avoid towards the forced direction.
	// 		const FVector RelVelNorm   = RelVel.GetSafeNormal();
	// 		const FVector::FReal Blend = FMath::Max(0., -FVector::DotProduct(ConNorm, RelVelNorm));
	// 		SeparationNormal           = FMath::Lerp(ConNorm, ForcedNormal, Blend).GetSafeNormal();
	// 	}
	//
	// 	const FVector::FReal StandingScaling = Collider.bIsMoving ? 1. : MovingAvoidanceParams.StandingObstacleAvoidanceScale;
	// 	// Care less about standing agents so that we can push through standing crowd.
	//
	// 	// Separation force (stay away from agents if possible)
	// 	const FVector::FReal PenSep        = (SeparationAgentRadius + Collider.Radius + MovingAvoidanceParams.ObstacleSeparationDistance) - ConDist;
	// 	const FVector::FReal SeparationMag = FMath::Square(FMath::Clamp(PenSep / MovingAvoidanceParams.ObstacleSeparationDistance, 0., 1.));
	// 	const FVector SepForce             = SeparationNormal * MovingAvoidanceParams.ObstacleSeparationStiffness;
	// 	const FVector SeparationForce      = SepForce * SeparationMag * StandingScaling;
	//
	// 	SteeringForce += SeparationForce;
	// 	TotalAgentSeparationForce += SeparationForce;
	//
	// 	// Calculate closest point of approach based on relative agent positions and velocities.
	// 	const FVector::FReal CPA = UE::MassAvoidance::ComputeClosestPointOfApproach(RelPos, RelVel, PredictiveAvoidanceAgentRadius + Collider.Radius,
	// 	                                                                            MovingAvoidanceParams.PredictiveAvoidanceTime);
	//
	// 	// Calculate penetration at CPA
	// 	const FVector AvoidRelPos      = RelPos + RelVel * CPA;
	// 	const FVector::FReal AvoidDist = AvoidRelPos.Size();
	// 	const FVector AvoidConNormal   = AvoidDist > 0. ? (AvoidRelPos / AvoidDist) : FVector::ForwardVector;
	//
	// 	FVector AvoidNormal = AvoidConNormal;
	// 	if (bHasForcedNormal)
	// 	{
	// 		// The more head on the predicted collisions is, the more we should avoid towards the forced direction.
	// 		const FVector RelVelNorm   = RelVel.GetSafeNormal();
	// 		const FVector::FReal Blend = FMath::Max(0., -FVector::DotProduct(AvoidConNormal, RelVelNorm));
	// 		AvoidNormal                = FMath::Lerp(AvoidConNormal, ForcedNormal, Blend).GetSafeNormal();
	// 	}
	//
	// 	const FVector::FReal AvoidPenetration = (PredictiveAvoidanceAgentRadius + Collider.Radius + MovingAvoidanceParams.PredictiveAvoidanceDistance) -
	// 		AvoidDist; // Based on future agents distance
	// 	const FVector::FReal AvoidMag = FMath::Square(FMath::Clamp(AvoidPenetration / MovingAvoidanceParams.PredictiveAvoidanceDistance, 0., 1.));
	// 	const FVector::FReal AvoidMagDist = (1. - (CPA * InvPredictiveAvoidanceTime)); // No clamp, CPA is between 0 and PredictiveAvoidanceTime
	// 	const FVector AvoidForce = AvoidNormal * AvoidMag * AvoidMagDist * MovingAvoidanceParams.ObstaclePredictiveAvoidanceStiffness * StandingScaling;
	//
	// 	SteeringForce += AvoidForce;
	// } // close entities loop
	//
	// SteeringForce *= NearStartScaling * NearEndScaling;
	//
	// Force.Value = UE::MassNavigation::ClampVector(SteeringForce, MaxSteerAccel); // Assume unit mass
}

FVector::FReal URVOProcessor::ComputeClosestPointOfApproach(const FVector RelPos, const FVector RelVel, const FVector::FReal TotalRadius,
	const FVector::FReal TimeHoriz)
{
	// Calculate time of impact based on relative agent positions and velocities.
	const FVector::FReal A = FVector::DotProduct(RelVel, RelVel);
	const FVector::FReal Inv2A = A > SMALL_NUMBER ? 1. / (2. * A) : 0.;
	const FVector::FReal B = FMath::Min(0., 2. * FVector::DotProduct(RelVel, RelPos));
	const FVector::FReal C = FVector::DotProduct(RelPos, RelPos) - FMath::Square(TotalRadius);
	// Using max() here gives us CPA (closest point on arrival) when there is no hit.
	const FVector::FReal Discr = FMath::Sqrt(FMath::Max(0., B * B - 4. * A * C));
	const FVector::FReal T = (-B - Discr) * Inv2A;
	return FMath::Clamp(T, 0., TimeHoriz);
}
