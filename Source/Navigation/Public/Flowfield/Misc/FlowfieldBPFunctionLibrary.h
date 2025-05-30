// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlowfieldBPFunctionLibrary.generated.h"

struct FGridSizes;
/**
 * 
 */
UCLASS()
class NAVIGATION_API UFlowfieldBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

	// Returns grid size in centimeters
	UFUNCTION(BlueprintCallable, Category = "Flowfield|Utilities")
	static FVector2D CalculateGridSize(const FGridSizes& GridSizes, const float& CellSize);
};
