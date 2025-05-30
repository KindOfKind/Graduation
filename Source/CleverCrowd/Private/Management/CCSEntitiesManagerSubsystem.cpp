// Fill out your copyright notice in the Description page of Project Settings.


#include "Management/CCSEntitiesManagerSubsystem.h"

#include "MassEntitySubsystem.h"
#include "MassSpawner.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Entity/EntityNotifierSubsystem.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "HashGrid/CCSEntitiesHashGrid.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/MovementFragments.h"

UCCSEntitiesManagerSubsystem::UCCSEntitiesManagerSubsystem()
{
	MovementSpeedsInClusters.Init(50.f, FCrowdStatistics::MaxClusterType + 1);
	AvoidanceRadiusInClusters.Init(120.f, FCrowdStatistics::MaxClusterType + 1);
	AvoidanceStrengthInClusters.Init(5.f, FCrowdStatistics::MaxClusterType + 1);
}

void UCCSEntitiesManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCCSEntitiesManagerSubsystem::PostInitialize()
{
	Super::PostInitialize();

	EntitiesHashGrid = NewObject<UCCSEntitiesHashGrid>(this);
	EntitiesHashGrid->EntityManager = GetEntityManager();

	CrowdStatistics = GetWorld()->GetSubsystem<UCrowdStatisticsSubsystem>();
	EntityNotifier  = GetWorld()->GetSubsystem<UEntityNotifierSubsystem>();
	EntityNotifier->PreDestroyEntityDelegate.BindLambda([this](const FMassEntityHandle& Entity)
	{
		FClusterFragment& ClusterFragment = EntityManager->GetFragmentDataChecked<FClusterFragment>(Entity);
		CrowdStatistics->Stats.ReachedFinishTimestamps.Add(GetWorld()->GetTimeSeconds());
		CrowdStatistics->Stats.RemoveAgentsCountInCluster(ClusterFragment.ClusterType, 1);
	});
}

void UCCSEntitiesManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

FMassEntityManager* UCCSEntitiesManagerSubsystem::GetEntityManager()
{
	if (EntityManager)
	{
		return EntityManager;
	}

	EntityManager = &GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	return EntityManager;
}

void UCCSEntitiesManagerSubsystem::SpawnAllAgents() const
{
	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMassSpawner::StaticClass(), SpawnerActors);
	for (int32 i = 0; i < SpawnerActors.Num(); i++)
	{
		Cast<AMassSpawner>(SpawnerActors[i])->DoSpawning();
	}
}


void UCCSEntitiesManagerSubsystem::SetDetourEnabledForEntitiesInBounds(const FGridBounds& Bounds, bool bEnabled)
{
	UGridUtilsFunctionLibrary::ForEachGridCell(Bounds, [this, bEnabled](const FGridCellPosition& CellPosition)
	{
		TArray<FMassEntityHandle> Entities;
		EntitiesHashGrid->GetEntitiesInCell(Entities, CellPosition);
		for (const FMassEntityHandle& Entity : Entities)
		{
			if (!EntityManager->IsEntityValid(Entity)) continue;
			FNavigationFragment* NavigationFragment = GetEntityManager()->GetFragmentDataPtr<FNavigationFragment>(Entity);
			if (!NavigationFragment) continue;
			NavigationFragment->bCanUseDetour = bEnabled;
		}
	});
}

void UCCSEntitiesManagerSubsystem::SetDetourEnabledForAllEntities(bool bEnabled)
{
	EntitiesHashGrid->ForEachNonEmptyCell([this, bEnabled](const FGridCellPosition& CellPosition, const TArray<FMassEntityHandle>& Entities,
		FMassEntityManager& EntityManager)
	{
		for (const FMassEntityHandle& Entity : Entities)
		{
			if (!EntityManager.IsEntityValid(Entity)) continue;
			FNavigationFragment* NavigationFragment = GetEntityManager()->GetFragmentDataPtr<FNavigationFragment>(Entity);
			if (!NavigationFragment) continue;
			NavigationFragment->bCanUseDetour = bEnabled;
		}
	});
}
