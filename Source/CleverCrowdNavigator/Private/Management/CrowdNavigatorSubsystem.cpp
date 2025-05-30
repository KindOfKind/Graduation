// Fill out your copyright notice in the Description page of Project Settings.


#include "Management/CrowdNavigatorSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassSpawner.h"
#include "MassSpawnerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/MovementFragments.h"

void UCrowdNavigatorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	EntityManager = &InWorld.GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();

	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(&InWorld, AMassSpawner::StaticClass(), SpawnerActors);
	for (int32 i = 0; i < SpawnerActors.Num(); i++)
	{
		MassSpawners.Add(Cast<AMassSpawner>(SpawnerActors[i]));
	}

	// Assign GoalPoint to spawned entities (Goal Point index is defined by the closest spawner)
	UMassSpawnerSubsystem* MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	MassSpawnerSubsystem->SpawnedEntitiesDelegate.BindLambda([this](const TArray<FMassEntityHandle>& SpawnedEntities)
	{
		for (const FMassEntityHandle& SpawnedEntity : SpawnedEntities)
		{
			float ShortestDistance = FLT_MAX;
			for (AMassSpawner* Spawner : MassSpawners)
			{
				FNavigationFragment& NavigationFragment = EntityManager->GetFragmentDataChecked<FNavigationFragment>(SpawnedEntity);
				FTransformFragment& TransformFragment   = EntityManager->GetFragmentDataChecked<FTransformFragment>(SpawnedEntity);
				FVector EntityLocation                  = TransformFragment.GetTransform().GetLocation();
				float Distance                          = FVector::Distance(Spawner->GetActorLocation(), EntityLocation);
				if (Distance < ShortestDistance)
				{
					ShortestDistance = Distance;
					NavigationFragment.GoalPointIndex = Spawner->GoalPointIndexTest;
				}
			}
		}
	});
}

void UCrowdNavigatorSubsystem::CacheDetourSearcher(FDetourSearcherRunnable* InDetourSearcher)
{
	DetourSearcher = InDetourSearcher;
}
