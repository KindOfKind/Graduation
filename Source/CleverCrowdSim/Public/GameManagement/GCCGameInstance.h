// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MapAnalyzer/MapAreaAnalyzer.h"
#include "GCCGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CLEVERCROWDSIM_API UGCCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	TMultiMap<int32, FMapAreaData> MapAreasDataConfigCached;	// Index - MapArea type id. Cached here to pass this data between tests.

public:
	bool LoadMapAreasConfigsFromFile();
	void SaveMapAreasConfigsToFile();
};
