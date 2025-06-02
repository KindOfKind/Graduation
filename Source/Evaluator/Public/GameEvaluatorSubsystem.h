// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CrowdEvaluationHashGrid.h"
#include "GameEvaluatorTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameEvaluatorSubsystem.generated.h"

class ACrowdEvaluationHashGrid;
class UCCSEntitiesManagerSubsystem;
class UCCSCollisionsSubsystem;
class FBufferArchive;
class UCrowdNavigatorSubsystem;
class UCrowdStatisticsSubsystem;
struct FMassEntityManager;
struct FMassEntityHandle;

/**
 * Subsystem that drives game tests on various maps, collects and saves statistics, toggles and tweaks crowd algorithms.
 */
UCLASS()
class EVALUATOR_API UGameEvaluatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	int32 GameSpeedDuringTest = 1;
	FString EvaluatorSavesPath;
	inline static FString MapAreasSubPath = "MapAreas/";

	TArray<FCrowdGroupAgentsStats> AveragedGroupAreasStatsCached;	// Cached to pass Crowd Groups info between tests during GameInstance lifetime

	bool bLoadedMapAreasConfigs = false;
	int32 MaxAreaTypeOnLevel = 0;

protected:
	FEvaluatorMetricParamsContainer MetricParams;        // Params that are calculated during tests to evaluate how well algorithms work
	FEvaluatorMetaParamsContainer MetaParams;            // Params that modify algorithms work and that are tweaked during tests to get the best metrics
	FCrowdAgentsEvaluationResult AgentsEvaluationResult; // Data about each agent at different moments of the simulation.
	
	// Data from previous tests
	TArray<FEvaluatorMetricParamsContainer> EvaluatedMetricParams;
	TArray<FEvaluatorMetaParamsContainer> EvaluatedMetaParams;
	
	// Array index - MapModificationIteration.
	// It currently has GameInstance storage time, is not saved into file and is cleared every time we proceed to MapModificationIteration 0.
	TArray<FCrowdAgentsEvaluationResult> AgentsEvaluationResultsByIterations;

	// Incremented after all preparations are finished. If 0, then the game is being prepared before the first test.
	int32 TestIteration = 0;
	// If bigger than 0, it means we want to make some modifications on the map and repeat the simulation with the same meta params.
	int32 MapModificationIteration = 0;

	FMassEntityManager* EntityManager;
	UPROPERTY()
	UWorld* World;
	UPROPERTY()
	TObjectPtr<UCrowdStatisticsSubsystem> CrowdStatisticsSubsystem;
	UPROPERTY()
	TObjectPtr<UCrowdNavigatorSubsystem> CrowdNavigator;
	UPROPERTY()
	TObjectPtr<UCCSCollisionsSubsystem> CollisionsSubsystem;
	UPROPERTY()
	TObjectPtr<UCCSEntitiesManagerSubsystem> EntityManagerSubsystem;
	UPROPERTY()
	TObjectPtr<ACrowdEvaluationHashGrid> EvaluationHashGrid;
	
	bool bPendingTestFinish = false;

public:
	void OnBeginPlay();
	void OnTick(float DeltaTime);
	void OnUpdatedEntitiesCount(const int32 NewCount, const int32 OldCount);

	void SaveTestData();
	void TransitionToRandomTestLevel();

	bool IsPendingTestFinish() const { return bPendingTestFinish; };
	void SetGameSpeed(int32 SpeedMult);

	const FEvaluatorMetaParamsContainer& GetMetaParams() const
	{
		return MetaParams;
	};

	FString GetEvaluatorSavesPath() const;
	FCrowdAgentsEvaluationResult& GetAgentsEvaluationResultMutable() { return AgentsEvaluationResult; };
	ACrowdEvaluationHashGrid* GetEvaluationHashGrid();
	bool IsEvaluatingMapAreas() const;

	FString GetSaveFilePath() const;	// Gets save file path for evaluation data for the current map
	bool WriteEvaluationDataToFileAsText();	// We can save the data as text to export it to external soft

private:
	void ResetEvaluationData();
	bool LoadEvaluationDataFromFile();
	bool WriteEvaluationDataToFile();
	void SaveData(FArchive& Ar);
	void LoadData(FArchive& Ar);

	void ModifyMetaParams();
	// @note: Some meta params are "taken" by other subsystems by default. It would be better to not use these params until ApplyMetaParams is called.
	void ApplyMetaParams() const;
};
