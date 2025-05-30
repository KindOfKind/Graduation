// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdNavigatorSubsystem.generated.h"


struct FMassEntityManager;
class AMassSpawner;
class FDetourSearcherRunnable;

UCLASS()
class CLEVERCROWDNAVIGATOR_API UCrowdNavigatorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	FMassEntityManager* EntityManager;
	FDetourSearcherRunnable* DetourSearcher;
	TArray<AMassSpawner*> MassSpawners;

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	void CacheDetourSearcher(FDetourSearcherRunnable* InDetourSearcher);
};
