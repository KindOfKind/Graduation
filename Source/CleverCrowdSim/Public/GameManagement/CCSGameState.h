// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CCSGameState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class CLEVERCROWDSIM_API ACCSGameState : public AGameState
{
	GENERATED_BODY()

private:

	bool bDebugMode;

public:

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugModeEnabled(const bool bEnableDebugMode) { bDebugMode = bEnableDebugMode; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Debug")
	bool IsDebugMode() const { return bDebugMode; }
};
