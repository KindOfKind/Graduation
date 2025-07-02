// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEvaluatorSubsystem.h"

#include "CrowdEvaluationHashGrid.h"
#include "MassEntitySubsystem.h"
#include "MassSpawner.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Management/CrowdNavigatorSubsystem.h"
#include "Management/CCSCollisionsSubsystem.h"
#include "Management/CCSEntitiesManagerSubsystem.h"


void UGameEvaluatorSubsystem::OnBeginPlay()
{
	EvaluatorSavesPath = FPaths::ProjectSavedDir() + "/SaveGame/Evaluator/";

	World = GetWorld();
	check(World);
	EntityManager            = &World->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	CrowdStatisticsSubsystem = World->GetSubsystem<UCrowdStatisticsSubsystem>();
	CrowdNavigator           = World->GetSubsystem<UCrowdNavigatorSubsystem>();
	CollisionsSubsystem      = World->GetSubsystem<UCCSCollisionsSubsystem>();
	EntityManagerSubsystem   = World->GetSubsystem<UCCSEntitiesManagerSubsystem>();
	bPendingTestFinish       = false;
	
	EvaluationHashGrid                          = GetEvaluationHashGrid();
	EvaluationHashGrid->GameEvaluator           = this;
	EvaluationHashGrid->AveragedGroupAreasStats = AveragedGroupAreasStatsCached;
	if (AveragedGroupAreasStatsCached.IsEmpty())
	{
		EvaluationHashGrid->AveragedGroupAreasStats.AddDefaulted(EvaluationHashGrid->MaxCrowdGroupTypes);
	}

	SetGameSpeed(GameSpeedDuringTest);
	UKismetSystemLibrary::ExecuteConsoleCommand(World, "t.MaxFPS = 999", nullptr);	// Remove FPS limit

	ResetEvaluationData();
	LoadEvaluationDataFromFile();
	ModifyMetaParams();
	ApplyMetaParams();

	EvaluationHashGrid->bAllowNewGroupTypesCreation = (TestIteration <= 0 && !bLoadedMapAreasConfigs);

	WriteEvaluationDataToFileAsText();

	// static constexpr TCHAR AvTickStr[] = TEXT("Previous average tick time: %f");
	// float AverageTick = MetricParams.AggregatedTickTime.Get().GetMean();
	// UKismetSystemLibrary::PrintString(World, FString::Printf(AvTickStr, AverageTick), true, true, FColor::Red, 50.f);
	//
	// static constexpr TCHAR TestDurationStr[] = TEXT("Previous test duration: %f");
	// UKismetSystemLibrary::PrintString(World, FString::Printf(TestDurationStr, MetricParams.TestDuration.Get()), true, true, FColor::Red, 50.f);
	//
	// static constexpr TCHAR DetourAddWeightStr[] = TEXT("Detour max additional weight: %d");
	// UKismetSystemLibrary::PrintString(World, FString::Printf(DetourAddWeightStr, MetaParams.DetourMaxAdditionalCost.Value), true, true, FColor::Red, 50.f);
	//
	// static constexpr TCHAR SaveFolderStr[] = TEXT("Save folder: %s");
	// UKismetSystemLibrary::PrintString(World, FString::Printf(SaveFolderStr, *EvaluatorSavesPath), true, true, FColor::Red, 50.f);

	
	MetricParams = FEvaluatorMetricParamsContainer{};
	TestIteration += 1;
}

void UGameEvaluatorSubsystem::OnTick(float DeltaTime)
{
	constexpr int32 ClustersNum = FCrowdStatistics::MaxClusterType;
	
	MetricParams.AggregatedTickTime.Get().AddValue(DeltaTime);
	
	MetricParams.AggregatedMassProcExecutionTime.Get().TotalValue = EntityManager->TotalProcessorsExecutionTime;
	MetricParams.AggregatedMassProcExecutionTime.Get().ValuesAmount += 1;

	FAggregatedValueFloat AggregatedSpeed;
	for (int32 ClusterType = 0; ClusterType < ClustersNum; ClusterType++)
	{
		float SpeedInCluster = CrowdStatisticsSubsystem->Stats.AverageEntitySpeedInClusters[ClusterType];
		MetricParams.AggregatedEntitiesMovementSpeedInClusters.GetInClusterChecked(ClusterType).AddValue(SpeedInCluster);
		if (SpeedInCluster > 0.0f)
		{
			AggregatedSpeed.AddValue(SpeedInCluster);
		}
	}
	MetricParams.AggregatedEntitiesMovementSpeed.Get().AddValue(AggregatedSpeed.GetMean());

	for (int32 AreaType = 0; AreaType <= MaxAreaTypeOnLevel; AreaType++)
	{
		float SpeedInArea = CrowdStatisticsSubsystem->Stats.AverageEntitySpeedInAreas[AreaType];
		if (!MetricParams.AggregatedEntitiesMovementSpeedAreal.IsValidAreaType(AreaType))
		{
			MetricParams.AggregatedEntitiesMovementSpeedAreal.SetValue(AreaType, FAggregatedValueFloat());
		}
		MetricParams.AggregatedEntitiesMovementSpeedAreal.GetValueMutable(AreaType).AddValue(SpeedInArea);
	}

	if (World->GetTimeSeconds() > 700.f)
	{
		bPendingTestFinish = true;
	}
}

void UGameEvaluatorSubsystem::OnUpdatedEntitiesCount(const int32 NewCount, const int32 OldCount)
{
	if (NewCount <= 10)
	{
		bPendingTestFinish = true;
	}
}

void UGameEvaluatorSubsystem::SaveTestData()
{
	constexpr int32 ClustersNum = FCrowdStatistics::MaxClusterType;
	
	MetricParams.TestDuration.Get() = World->GetTimeSeconds();

	for (int32 ClusterType = 0; ClusterType < ClustersNum; ClusterType++)
	{
		MetricParams.CollisionsCountInClusters.GetInClusterChecked(ClusterType) = CrowdStatisticsSubsystem->Stats.CollisionsInClusters[ClusterType];
	}
	for (int32 AreaType = 0; AreaType <= MaxAreaTypeOnLevel; AreaType++)
	{
		MetricParams.CollisionsCountAreal.SetValue(AreaType, CrowdStatisticsSubsystem->Stats.CollisionsInAreas[AreaType]);
	}

	FAggregatedValueFloat AggregatedFinishTime;
	for (float EntityFinishTime : CrowdStatisticsSubsystem->Stats.ReachedFinishTimestamps)
	{
		AggregatedFinishTime.AddValue(EntityFinishTime);
	}
	MetricParams.AverageEntityFinishedTime.Get() = AggregatedFinishTime.GetMean();
	
	for (int32 AreaType = 0; AreaType <= MaxAreaTypeOnLevel; AreaType++)
	{
		float AverageTimeInArea = 0.f;
		TMap<FMassEntityHandle, float>* EntitiesTotalTimeMap = CrowdStatisticsSubsystem->Stats.EntitiesTotalTimeInArea.Find(AreaType);
		if (EntitiesTotalTimeMap)
		{
			int32 EntitiesInAreaNum = EntitiesTotalTimeMap->Num();
			for (auto& [Entity, TotalTime] : *EntitiesTotalTimeMap)
			{
				AverageTimeInArea += (TotalTime / EntitiesInAreaNum);
			}
		}
		MetricParams.AverageEntityTimeInAreas.SetValue(AreaType, AverageTimeInArea);
	}
	
	EvaluatedMetricParams.Add(MetricParams);
	EvaluatedMetaParams.Add(MetaParams);
	WriteEvaluationDataToFile();
}

void UGameEvaluatorSubsystem::TransitionToRandomTestLevel()
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(World);
	UGameplayStatics::OpenLevel(World, *LevelName, true);	// For now we open the same level
}

void UGameEvaluatorSubsystem::SetGameSpeed(int32 SpeedMult)
{
	const FString Cmd = "slomo " + FString::FromInt(SpeedMult);
	UKismetSystemLibrary::ExecuteConsoleCommand(World, Cmd, nullptr);
}

FString UGameEvaluatorSubsystem::GetEvaluatorSavesPath() const
{
	return FPaths::ProjectSavedDir() + "/SaveGame/Evaluator/";
}

ACrowdEvaluationHashGrid* UGameEvaluatorSubsystem::GetEvaluationHashGrid()
{
	if (!IsValid(EvaluationHashGrid))
	{
		FActorSpawnParameters HashGridSpawnParameters;
		HashGridSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		EvaluationHashGrid = GetWorld()->SpawnActor<ACrowdEvaluationHashGrid>(HashGridSpawnParameters);
	}
	return EvaluationHashGrid;
}

bool UGameEvaluatorSubsystem::IsEvaluatingMapAreas() const
{
	return (TestIteration <= 1 && !bLoadedMapAreasConfigs);
}

void UGameEvaluatorSubsystem::ResetEvaluationData()
{
	MetricParams = FEvaluatorMetricParamsContainer{};
	MetaParams = FEvaluatorMetaParamsContainer{};
	EvaluatedMetricParams.Reset();
	EvaluatedMetaParams.Reset();
	AgentsEvaluationResult.Clear();
}

bool UGameEvaluatorSubsystem::LoadEvaluationDataFromFile()
{
	TArray<uint8> BinData;

	const FString SaveFilePath = GetSaveFilePath();
	if (!FPaths::FileExists(SaveFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] Save file doesn't exist."), __FUNCTION__);
		return false;
	}
	if (!FFileHelper::LoadFileToArray(BinData, *SaveFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] Failed to load save from file."), __FUNCTION__);
		return false;
	}
	if(BinData.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] No data has been loaded from file."), __FUNCTION__);
		return false;
	}
	
	FMemoryReader Ar = FMemoryReader(BinData, true);
	Ar.Seek(0);
	LoadData(Ar);

	if (!EvaluatedMetaParams.IsEmpty())
	{
		MetaParams = EvaluatedMetaParams.Last();
	}

	Ar.FlushCache();
	BinData.Empty();
	Ar.Close();
	return true;
}

bool UGameEvaluatorSubsystem::WriteEvaluationDataToFile()
{
	TArray<uint8> BinData;
	FMemoryWriter Ar = FMemoryWriter(BinData, true);
	SaveData(Ar);

	if(BinData.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] No data to save."), __FUNCTION__);
		return false;
	}

	const FString SaveFilePath = GetSaveFilePath();
	if (FFileHelper::SaveArrayToFile(BinData, *SaveFilePath))
	{
		Ar.FlushCache();
		BinData.Empty();
		UE_LOG(LogTemp, Display, TEXT("[%hs] Saved evaluated data."), __FUNCTION__);
		return true;
	}
	
	return false;
}

void UGameEvaluatorSubsystem::SaveData(FArchive& Ar)
{
	int32 Num = 0;
	
	Num = EvaluatedMetricParams.Num();
	Ar << Num;
	for (int32 i = 0; i < Num; i++)
	{
		Ar << EvaluatedMetricParams[i];
	}
	
	Num = EvaluatedMetaParams.Num();
	Ar << Num;
	for (int32 i = 0; i < Num; i++)
	{
		Ar << EvaluatedMetaParams[i];
	}
}

void UGameEvaluatorSubsystem::LoadData(FArchive& Ar)
{
	int32 Num = 0;
	
	Ar << Num;
	for (int32 i = 0; i < Num; i++)
	{
		EvaluatedMetricParams.AddDefaulted();
		Ar << EvaluatedMetricParams.Last();
	}

	Ar << Num;
	for (int32 i = 0; i < Num; i++)
	{
		EvaluatedMetaParams.AddDefaulted();
		Ar << EvaluatedMetaParams.Last();
	}
}

FString UGameEvaluatorSubsystem::GetSaveFilePath() const
{
	const FString CurrentLevelName = World->GetMapName();
	return EvaluatorSavesPath + CurrentLevelName + "_" + FString::FromInt(MapModificationIteration) + ".txt";
}

bool UGameEvaluatorSubsystem::WriteEvaluationDataToFileAsText()
{
	if (EvaluatedMetaParams.IsEmpty() || EvaluatedMetricParams.IsEmpty())
	{
		return false;
	}
	
	// Write metric params ------
	FString Str;
	EvaluatedMetricParams[0].WriteParamsNamesIntoString(Str);	// We consider that the header will match metrics from all tests on the current map
	
	int32 Num = EvaluatedMetricParams.Num();
	for (int32 i = 0; i < Num; i++)
	{
		EvaluatedMetricParams[i].WriteParamsValuesIntoString(Str);
	}

	const FString MetricsSaveFilePath = EvaluatorSavesPath + World->GetMapName() + "_Metrics_AsText.txt";
	if (FFileHelper::SaveStringToFile(Str, *MetricsSaveFilePath))
	{
		UE_LOG(LogTemp, Display, TEXT("[%hs] Saved evaluated metrics data as text."), __FUNCTION__);
	}

	// Write meta params ------
	Str = "";
	EvaluatedMetaParams[0].WriteParamsNamesIntoString(Str);
	
	Num = EvaluatedMetaParams.Num();
	for (int32 i = 0; i < Num; i++)
	{
		EvaluatedMetaParams[i].WriteParamsValuesIntoString(Str);
	}

	const FString MetaSaveFilePath = EvaluatorSavesPath + World->GetMapName() + "_Meta_AsText.txt";
	if (FFileHelper::SaveStringToFile(Str, *MetaSaveFilePath))
	{
		UE_LOG(LogTemp, Display, TEXT("[%hs] Saved evaluated meta data as text."), __FUNCTION__);
	}
	
	return false;
}

void UGameEvaluatorSubsystem::ModifyMetaParams()
{
	MetaParams.PrepareArealParams(MaxAreaTypeOnLevel);
	MetaParams.RandomizeAllParams();
	MetaParams.TestsNum += 1;

	// We don't want to use avoidance during the first test (when we configure areas) 
	if (TestIteration == 0 && !bLoadedMapAreasConfigs)
	{
		for (TEvaluatorMetaParam<float>& MetaParam : MetaParams.AvoidanceStrengthAreal.Get())
		{
			MetaParam.Value = 0.f;
		}
		MetaParams.AvoidanceStrengthSpaciousCluster.Value = 0.f;
		
		for (TEvaluatorMetaParam<float>& MetaParam : MetaParams.ToTheSideAvoidanceDurationAreal.Get())
		{
			MetaParam.Value = 0.f;
		}
		MetaParams.DefaultToTheSideAvoidanceDuration.Value = 0.f;
	}
}

void UGameEvaluatorSubsystem::ApplyMetaParams() const
{
	CollisionsSubsystem->GetCollisionsHashGrid().DecrementCollisionsCountRate = MetaParams.DecrementCollisionsCountRate.Value;

	EntityManagerSubsystem->MovementSpeedsInClusters[CCSCrowdCluster::ClusterTypes::Undefined] = MetaParams.AgentMovementSpeedSpaciousCluster.Value;
	EntityManagerSubsystem->MovementSpeedsInClusters[CCSCrowdCluster::ClusterTypes::Spacious] = MetaParams.AgentMovementSpeedSpaciousCluster.Value;
	EntityManagerSubsystem->MovementSpeedsInClusters[CCSCrowdCluster::ClusterTypes::Dense] = MetaParams.AgentMovementSpeedDenseCluster.Value;
	for (TEvaluatorMetaParam<float> MetaParam : MetaParams.AgentMovementSpeedAreal.Get())
	{
		EntityManagerSubsystem->MovementSpeedsInAreas.Add(MetaParam.Value);
	}

	// Spawn agents uniformly from all spawners
	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(World, AMassSpawner::StaticClass(), SpawnerActors);
	const int32 AgentsPerSpawner = MetaParams.AgentsNum.Value / SpawnerActors.Num();
	for (int32 i = 0; i < SpawnerActors.Num(); i++)
	{
		Cast<AMassSpawner>(SpawnerActors[i])->SpawnCountOverride = AgentsPerSpawner;
	}

	EntityManagerSubsystem->AvoidanceStrengthInClusters[CCSCrowdCluster::ClusterTypes::Undefined] = MetaParams.AvoidanceStrengthSpaciousCluster.Value;
	EntityManagerSubsystem->AvoidanceStrengthInClusters[CCSCrowdCluster::ClusterTypes::Spacious] = MetaParams.AvoidanceStrengthSpaciousCluster.Value;
	EntityManagerSubsystem->AvoidanceStrengthInClusters[CCSCrowdCluster::ClusterTypes::Dense] = MetaParams.AvoidanceStrengthDenseCluster.Value;
	for (TEvaluatorMetaParam<float> MetaParam : MetaParams.AvoidanceStrengthAreal.Get())
	{
		EntityManagerSubsystem->AvoidanceStrengthInAreas.Add(MetaParam.Value);
	}

	EntityManagerSubsystem->AvoidanceRadiusInClusters[CCSCrowdCluster::ClusterTypes::Undefined] = MetaParams.AvoidanceRadiusSpaciousCluster.Value;
	EntityManagerSubsystem->AvoidanceRadiusInClusters[CCSCrowdCluster::ClusterTypes::Spacious] = MetaParams.AvoidanceRadiusSpaciousCluster.Value;
	EntityManagerSubsystem->AvoidanceRadiusInClusters[CCSCrowdCluster::ClusterTypes::Dense] = MetaParams.AvoidanceRadiusDenseCluster.Value;
	for (TEvaluatorMetaParam<float> MetaParam : MetaParams.AvoidanceRadiusAreal.Get())
	{
		EntityManagerSubsystem->AvoidanceRadiusInAreas.Add(MetaParam.Value);
	}

	EntityManagerSubsystem->DefaultToTheSideAvoidanceDuration = MetaParams.DefaultToTheSideAvoidanceDuration.Value;
	EntityManagerSubsystem->AvoidanceType = MetaParams.AvoidanceType.Value;
	for (TEvaluatorMetaParam<float> MetaParam : MetaParams.ToTheSideAvoidanceDurationAreal.Get())
	{
		EntityManagerSubsystem->ToTheSideAvoidanceDurationInAreas.Add(MetaParam.Value);
	}
}
