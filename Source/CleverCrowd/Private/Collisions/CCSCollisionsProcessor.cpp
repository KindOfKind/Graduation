// Fill out your copyright notice in the Description page of Project Settings.


#include "Collisions/CCSCollisionsProcessor.h"

#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "Collisions/CollisionsFragments.h"
#include "Collisions/Obstacles/CCSObstaclesHashGrid.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "HashGrid/CCSEntitiesHashGrid.h"
#include "Kismet/KismetMathLibrary.h"
#include "Management/CCSCollisionsSubsystem.h"
#include "Management/CCSEntitiesManagerSubsystem.h"
#include "Global/CrowdStatisticsSubsystem.h"

UCCSCollisionsProcessor::UCCSCollisionsProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	bRequiresGameThreadExecution = true;
}

void UCCSCollisionsProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadOnly);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCCSCollisionsProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	EntitiesHashGrid         = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>()->GetEntitiesHashGrid();
	CollisionsSubsystem      = GetWorld()->GetSubsystem<UCCSCollisionsSubsystem>();
	CrowdStatisticsSubsystem = GetWorld()->GetSubsystem<UCrowdStatisticsSubsystem>();
}

void UCCSCollisionsProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	constexpr int ClusterSize = 2;
	constexpr float MinCollisionCountRateForEntity = 2.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&, this](FMassExecutionContext& Context)
	{
		const int32 NumEntities                            = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FCollisionFragment> CollisionList = Context.GetMutableFragmentView<FCollisionFragment>();

		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FCollisionFragment& CollisionFragment = CollisionList[EntityIndex];

			CollisionFragment.bCollidedAtPreviousTick = false;
		}
	});

	
	EntitiesHashGrid->ParallelForEachNonEmptyCell([this, &EntityManager, CurrentTime](const FGridCellPosition& Cell)
	{
		TArray<FMassEntityHandle> Entities;
		FGridBounds Bounds{Cell, Cell + FGridCellPosition{ClusterSize - 1, ClusterSize - 1}};
		EntitiesHashGrid->GetEntitiesInBounds(Entities, Bounds);	// Getting entities from 4 adjacent grid cells
		
		const int32 EntitiesNum = Entities.Num();
		for (int32 EntityIndex = 0; EntityIndex < EntitiesNum; ++EntityIndex)
		{
			if (!EntityManager.IsEntityValid(Entities[EntityIndex]))
			{
				continue;
			}
			
			FTransform& Transform                 = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[EntityIndex]).GetMutableTransform();
			const float Radius                    = EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entities[EntityIndex]).Radius;
			FCollisionFragment& CollisionFragment = EntityManager.GetFragmentDataChecked<FCollisionFragment>(Entities[EntityIndex]);
			FClusterFragment& ClusterFragment     = EntityManager.GetFragmentDataChecked<FClusterFragment>(Entities[EntityIndex]);
			const FVector& Location               = Transform.GetLocation();
			bool bCollided                        = false;

			FEntityProxyData EntityData {
				Transform, Location, Radius, CollisionFragment
			};

			for (int32 OtherEntityIndex = 0; OtherEntityIndex < EntitiesNum; ++OtherEntityIndex)
			{
				if (!EntityManager.IsEntityValid(Entities[OtherEntityIndex]) || Entities[EntityIndex] == Entities[OtherEntityIndex])
				{
					continue;
				}
				
				FTransform& OtherTransform   = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[OtherEntityIndex]).GetMutableTransform();
				const float OtherRadius      = EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(Entities[OtherEntityIndex]).Radius;
				FCollisionFragment& OtherCollisionFragment = EntityManager.GetFragmentDataChecked<FCollisionFragment>(Entities[OtherEntityIndex]);
				const FVector& OtherLocation = OtherTransform.GetLocation();
				FVector OtherLocation2D      = FVector{OtherLocation.X, OtherLocation.Y, 0.f};

				FEntityProxyData OtherEntityData {
					OtherTransform, OtherLocation, OtherRadius, OtherCollisionFragment
				};

				bool bLocalCollisionOccured = false;
				ResolveTwoAgentsCollisions(bLocalCollisionOccured, EntityData, OtherEntityData);
				bCollided = bCollided || bLocalCollisionOccured;

				if (bLocalCollisionOccured)
				{
					CollisionFragment.bCollidedAtPreviousTick      = true;
					OtherCollisionFragment.bCollidedAtPreviousTick = true;
				}
			}

			if (bCollided)
			{
				if (CurrentTime - CollisionFragment.LastCollisionCountTime > MinCollisionCountRateForEntity)
				{
					CrowdStatisticsSubsystem->Stats.CollisionsInClusters[ClusterFragment.ClusterType] += 1;
					if (ClusterFragment.AreaId > INDEX_NONE)
					{
						CrowdStatisticsSubsystem->Stats.CollisionsInAreas[ClusterFragment.AreaId] += 1;
					}
					CollisionFragment.LastCollisionCountTime = CurrentTime;
					TryIncreaseCollisionsCount(bCollided, Location);
				}
			}
			
			ResolveAgentCollisionsWithObstacles(EntityData);
		}
	});
}

void UCCSCollisionsProcessor::ResolveTwoAgentsCollisions(bool& bOutCollisionOccured, FEntityProxyData& EntityData, FEntityProxyData& OtherEntityData)
{
	FVector Location2D      = FVector{EntityData.Location.X, EntityData.Location.Y, 0.f};
	FVector OtherLocation2D = FVector{OtherEntityData.Location.X, OtherEntityData.Location.Y, 0.f};
	
	const float CollisionResolveDistance = ((EntityData.Radius + OtherEntityData.Radius) - FVector::Distance(Location2D, OtherLocation2D)) / 2.f;
	if (const bool bCollisionsOverlap = (CollisionResolveDistance > 0.f))
	{
		bOutCollisionOccured = true;
		
		if (Location2D == OtherLocation2D)
		{
			EntityData.Transform.SetLocation(EntityData.Location + FMath::VRand());
			OtherEntityData.Transform.SetLocation(OtherEntityData.Location - FMath::VRand());
		}
		else
		{
			const FVector CollisionResolveDirection = UKismetMathLibrary::GetDirectionUnitVector(Location2D, OtherLocation2D);
			EntityData.Transform.SetLocation(EntityData.Location - CollisionResolveDirection * CollisionResolveDistance);
			OtherEntityData.Transform.SetLocation(OtherEntityData.Location + CollisionResolveDirection * CollisionResolveDistance);

			if (CollisionResolveDistance > 5.f)	// Count the collision if penetration was big enough
			{
				//DrawDebugVerticalLineInGameThread(GetWorld(), Location2D, .4f);
				EntityData.CollisionFragment.StrongCollisionsCounterMeta += 1;
			}
			else if (CollisionResolveDistance > 3.3f)
			{
				//DrawDebugVerticalLineInGameThread(GetWorld(), Location2D, .4f);
				EntityData.CollisionFragment.WeakCollisionsCounterMeta += 1;
			}
		}
	}
}

void UCCSCollisionsProcessor::ResolveAgentCollisionsWithObstacles(FEntityProxyData& EntityData)
{
	TArray<FCCSObstacleEdge> ObstacleEdges;	// All nearby obstacle edges
	CollisionsSubsystem->GetObstaclesHashGrid()->GetObstacleEdgesAtLocation(ObstacleEdges, EntityData.Location, EntityData.Radius);

	for (const FCCSObstacleEdge& Edge : ObstacleEdges)
	{
		FVector EdgeStart = FVector{Edge.Start.X, Edge.Start.Y, EntityData.Location.Z};
		FVector EdgeEnd   = FVector{Edge.End.X, Edge.End.Y, EntityData.Location.Z};

		const FVector ClosestPointOnEdge = FMath::ClosestPointOnSegment(EntityData.Location, EdgeStart, EdgeEnd);
		const float DistanceToEdge       = FVector::Distance(EntityData.Location, ClosestPointOnEdge);

		if (DistanceToEdge < EntityData.Radius)
		{
			const float ResolveDistance      = EntityData.Radius - DistanceToEdge;
			const FVector ResolveDirection   = Edge.LeftDir;
			const FVector ResolveTranslation = ResolveDistance * ResolveDirection;

			const FVector NewLocation = EntityData.Location + FVector{ResolveTranslation.X, ResolveTranslation.Y, 0.f};
			EntityData.Transform.SetLocation(NewLocation);
		}
	}
}


void UCCSCollisionsProcessor::TryIncreaseCollisionsCount(bool bCollisionOccured, const FVector& Location) const
{
	if (!bCollisionOccured)
	{
		return;
	}
	CollisionsSubsystem->GetCollisionsHashGrid().AddCollisionsCountAtLocation(Location, 1);
}


// DEBUG ------

void UCCSCollisionsProcessor::DrawDebugVerticalLineInGameThread(UWorld* World, const FVector& Location, const float LifeTime)
{
	AsyncTask(ENamedThreads::Type::GameThread, [World, Location, LifeTime]()
	{
		DrawDebugLine(World, Location, Location + FVector::UpVector * 500.f, FColor::Red, false, LifeTime, 0, 2.f);
	});
}
