// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Collisions/CCSCollisionsHashGrid.h"
#include "Subsystems/WorldSubsystem.h"
#include "CCSCollisionsSubsystem.generated.h"

class UCCSObstaclesHashGrid;

UCLASS()
class CLEVERCROWD_API UCCSCollisionsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
private:

	UPROPERTY()
	TObjectPtr<UCCSObstaclesHashGrid> ObstaclesHashGrid;
	FCCSCollisionsHashGrid CollisionsHashGrid;

	FTimerHandle DebugCollisionsCountTh;

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:

	void InitializeObstaclesOnMap();

	UCCSObstaclesHashGrid* GetObstaclesHashGrid() { return ObstaclesHashGrid; };
	FCCSCollisionsHashGrid& GetCollisionsHashGrid() { return CollisionsHashGrid; };

private:
	void DrawDebugCollisionsCountsPeriodically();
};
