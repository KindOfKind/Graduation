// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CCSPlayerController.h"

#include "CrowdEvaluationHashGrid.h"
#include "GameEvaluatorSubsystem.h"
#include "GameManagement/GCCGameInstance.h"
#include "Kismet/GameplayStatics.h"


void ACCSPlayerController::ClearSaves(bool bClearParams, bool bClearMapAreas, int32 Mode)
{
	UWorld* World = GetWorld();
	UGameEvaluatorSubsystem* GameEvaluator = World->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	check(GameEvaluator);
	FString FolderToClear = GameEvaluator->EvaluatorSavesPath;
	IFileManager::Get().IterateDirectory(*FolderToClear, [bClearParams, bClearMapAreas](const TCHAR* Pathname, bool bIsDirectory)
	{
		if (bIsDirectory)
		{
			if (bClearMapAreas && FString(Pathname).Contains(TEXT("/MapAreas")))
			{
				IFileManager::Get().DeleteDirectory(Pathname, true, true);
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (bClearParams)
			{
				IFileManager::Get().Delete(Pathname);
			}
		}
		return true;
	});

	if (bClearMapAreas)
	{
		UGCCGameInstance* GCCGameInstance = Cast<UGCCGameInstance>(World->GetGameInstance());
		GCCGameInstance->MapAreasDataConfigCached.Empty();
	}

	// Finish game (restart won't do and will break some simulation processes)
	UKismetSystemLibrary::QuitGame(World, nullptr, EQuitPreference::Quit, true);
}

void ACCSPlayerController::CCS_ClearSavedData(int32 Mode)
{
	ClearSaves(true, true, Mode);
}

void ACCSPlayerController::CCS_ClearMetaAndMetricsSaves(int32 Mode)
{
	ClearSaves(true, false, Mode);
}

void ACCSPlayerController::CCS_SaveMapAreasData()
{
	UGCCGameInstance* GCCGameInstance = Cast<UGCCGameInstance>(GetWorld()->GetGameInstance());
	GCCGameInstance->SaveMapAreasConfigsToFile();
}

void ACCSPlayerController::CCS_SaveEvaluationData()
{
	UGameEvaluatorSubsystem* GameEvaluator = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	GameEvaluator->SaveTestData();
	GameEvaluator->WriteEvaluationDataToFileAsText();
}

void ACCSPlayerController::DebugDrawCrowdGroupAreasAveraged()
{
	GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>()->GetEvaluationHashGrid()->DebugDrawCrowdGroupAreasAveraged(GetWorld(), 5.f, 15.f);
}
