// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManagement/GCCGameInstance.h"

#include "GameEvaluatorSubsystem.h"
UE_DISABLE_OPTIMIZATION
bool UGCCGameInstance::LoadMapAreasConfigsFromFile()
{
	UGameEvaluatorSubsystem* GameEvaluator = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	check(GameEvaluator);
	FString SaveFilePath = GameEvaluator->GetEvaluatorSavesPath() + GameEvaluator->MapAreasSubPath + "Default.txt";

	TArray<uint8> BinData;

	if (!FPaths::FileExists(SaveFilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("[%hs] Save file doesn't exist."), __FUNCTION__);
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

	MapAreasDataConfigCached.Empty();
	int32 Num = 0;
	Ar << Num;
	for (int32 i = 0; i < Num; i++)
	{
		int32 MapAreaTypeId;
		FMapAreaData MapAreaData;
		Ar << MapAreaTypeId;
		Ar << MapAreaData;
		MapAreasDataConfigCached.Add(MapAreaTypeId, MapAreaData);
	}
	
	GameEvaluator->AveragedGroupAreasStatsCached.Empty();
	Ar << GameEvaluator->AveragedGroupAreasStatsCached;

	GameEvaluator->bLoadedMapAreasConfigs = !MapAreasDataConfigCached.IsEmpty();

	Ar.FlushCache();
	BinData.Empty();
	Ar.Close();
	
	return true;
}

void UGCCGameInstance::SaveMapAreasConfigsToFile()
{
	UGameEvaluatorSubsystem* GameEvaluator = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	check(GameEvaluator);
	FString SaveFilePath = GameEvaluator->GetEvaluatorSavesPath() + GameEvaluator->MapAreasSubPath + "Default.txt";
	
	TArray<uint8> BinData;
	FMemoryWriter Ar = FMemoryWriter(BinData, true);
	
	int32 Num = MapAreasDataConfigCached.Num();
	Ar << Num;
	for (auto& [MapAreaTypeId, MapAreaData] : MapAreasDataConfigCached)
	{
		Ar << MapAreaTypeId;
		Ar << MapAreaData;
	}
	Ar << GameEvaluator->AveragedGroupAreasStatsCached;

	if(BinData.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] No data to save."), __FUNCTION__);
		return;
	}

	if (FFileHelper::SaveArrayToFile(BinData, *SaveFilePath))
	{
		Ar.FlushCache();
		BinData.Empty();
		UE_LOG(LogTemp, Display, TEXT("[%hs] Saved evaluated data."), __FUNCTION__);
		return;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] Failed to save Map Areas data."), __FUNCTION__);
	}
}
UE_ENABLE_OPTIMIZATION