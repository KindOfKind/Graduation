// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Flowfield/Detour/DetourSearcherRunnable.h"
#include "GameFramework/GameMode.h"
#include "Management/CCSCollisionsSubsystem.h"
#include "CCSGameMode.generated.h"


class UGCCGameInstance;
class UCrowdNavigatorSubsystem;
class UCrowdStatisticsSubsystem;
class UGameEvaluatorSubsystem;
class UEntityNotifierSubsystem;
class UMapAnalyzerSubsystem;
class UCCSEntitiesManagerSubsystem;
class AFlowfield;
class UCrowdNavigationSubsystem;

UCLASS(Blueprintable)
class CLEVERCROWDSIM_API ACCSGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	float DetoursSearchRate = 8.f;
	bool bDisableDetour = false;

private:

	// CACHED ------

	TObjectPtr<UGCCGameInstance> GCCGameInstance;
	TObjectPtr<UCrowdNavigationSubsystem> CrowdNavigationSubsystem;
	TObjectPtr<UCCSCollisionsSubsystem> CollisionsSubsystem;
	TObjectPtr<UMapAnalyzerSubsystem> MapAnalyzerSubsystem;
	TObjectPtr<UCCSEntitiesManagerSubsystem> EntitiesManagerSubsystem;
	TObjectPtr<UEntityNotifierSubsystem> EntityNotifier;
	TObjectPtr<UGameEvaluatorSubsystem> GameEvaluator;
	TObjectPtr<UCrowdStatisticsSubsystem> CrowdStatisticsSubsystem;

	TObjectPtr<UCrowdNavigatorSubsystem> CrowdNavigator;
	FDetourSearcherRunnable* DetourSearcherRunnable;

	FTimerHandle DetourRecalculationTh;

	float DestroyTime = 900.f;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flowfield")
	AFlowfield* GetFlowfield();

private:

	void InitDetoursSearcher();
	void ApplyDetourDirectionsGrids(TArray<TSharedPtr<FDirectionsGrid>> DetourDirectionsGrids, TArray<FGridBounds> DenseAreas);
};
