// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CCSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CLEVERCROWDSIM_API ACCSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void CCS_ClearSavedData(int32 Mode = -1);
	UFUNCTION(Exec)
	void CCS_ClearMetaAndMetricsSaves(int32 Mode = -1);

	UFUNCTION(Exec)
	void CCS_SaveMapAreasData();
	UFUNCTION(Exec)
	void CCS_SaveEvaluationData();
	
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugDrawCrowdGroupAreasAveraged();

private:
	void ClearSaves(bool bClearParams, bool bClearMapAreas, int32 Mode = -1);
};
