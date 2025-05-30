// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CCSPlayerController.h"

#include "CrowdEvaluationHashGrid.h"
#include "GameEvaluatorSubsystem.h"
#include "GameManagement/GCCGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ACCSPlayerController::CCS_ClearSavedData(int32 Mode)
{
	UWorld* World = GetWorld();
	UGameEvaluatorSubsystem* GameEvaluator = World->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	check(GameEvaluator);
	FString FolderToClear = GameEvaluator->EvaluatorSavesPath;
	IFileManager::Get().IterateDirectory(*FolderToClear, [](const TCHAR* Pathname, bool bIsDirectory)
	{
		if (bIsDirectory)
		{
			if (!FString(Pathname).Contains(TEXT("/MapAreas")))
			{
				return true;
			}
			IFileManager::Get().DeleteDirectory(Pathname, true, true);
		}
		else
		{
			IFileManager::Get().Delete(Pathname);
		}
		return true;
	});

	// Clear cached variables and finish game (restart won't do and will break some simulation processes)
	UGCCGameInstance* GCCGameInstance = Cast<UGCCGameInstance>(World->GetGameInstance());
	GCCGameInstance->MapAreasDataConfigCached.Empty();
	
	UKismetSystemLibrary::QuitGame(World, nullptr, EQuitPreference::Quit, true);
}

void ACCSPlayerController::CCS_SaveMapAreasData()
{
	UGCCGameInstance* GCCGameInstance = Cast<UGCCGameInstance>(GetWorld()->GetGameInstance());
	GCCGameInstance->SaveMapAreasConfigsToFile();
}

void ACCSPlayerController::DebugDrawCrowdGroupAreasAveraged()
{
	GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>()->GetEvaluationHashGrid()->DebugDrawCrowdGroupAreasAveraged(GetWorld(), 5.f, 15.f);
}
