// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CCSEntitiesManagerSubsystem.generated.h"

class UCrowdStatisticsSubsystem;
class UEntityNotifierSubsystem;
struct FGridBounds;
struct FMassEntityManager;
class UCCSEntitiesHashGrid;
/**
 * 
 */
UCLASS()
class CLEVERCROWD_API UCCSEntitiesManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ToDo: Refactor this temporary solution
	TArray<float> MovementSpeedsInClusters;
	TArray<float> MovementSpeedsInAreas;

	TArray<float> AvoidanceStrengthInClusters;
	TArray<float> AvoidanceStrengthInAreas;
	TArray<float> AvoidanceRadiusInClusters;
	TArray<float> AvoidanceRadiusInAreas;
	
	TArray<float> ToTheSideAvoidanceDurationInAreas;
	float DefaultToTheSideAvoidanceDuration;
	int32 AvoidanceType;

private:
	UPROPERTY()
	TObjectPtr<UCCSEntitiesHashGrid> EntitiesHashGrid;
	UPROPERTY()
	TObjectPtr<UEntityNotifierSubsystem> EntityNotifier;
	UPROPERTY()
	TObjectPtr<UCrowdStatisticsSubsystem> CrowdStatistics;

	FMassEntityManager* EntityManager;

public:
	UCCSEntitiesManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PostInitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	FMassEntityManager* GetEntityManager();
	UCCSEntitiesHashGrid* GetEntitiesHashGrid() { return EntitiesHashGrid; };

	void SpawnAllAgents() const;

	// DETOUR START ------
	void SetDetourEnabledForEntitiesInBounds(const FGridBounds& Bounds, bool bEnabled);
	void SetDetourEnabledForAllEntities(bool bEnabled);
	// DETOUR END
};
