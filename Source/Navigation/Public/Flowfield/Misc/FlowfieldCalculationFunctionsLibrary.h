// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FlowfieldCalculationFunctionsLibrary.generated.h"


struct FIntegrationGrid;
struct FGridBounds;
struct FGridSizes;
struct FGridCellPosition;
struct FCostsGrid;
struct FDirectionsGrid;

UCLASS()
class NAVIGATION_API UFlowfieldCalculationFunctionsLibrary : public UObject
{
	GENERATED_BODY()

public:

	// @note OutIntegrationGrid should be initialized from CostsGrid before passing to this method
	static void CalculateIntegrationGrid(FIntegrationGrid& OutIntegrationGrid, const FGridCellPosition& GoalCellPosition, const FGridBounds& GridBounds, const FCostsGrid& CostsGrid);
	static void CalculateDirectionsGrid(FDirectionsGrid& OutDirectionsGrid, const FCostsGrid& CostsGrid, const FGridCellPosition& GoalCellPosition, const FGridSizes& GridSizes);

	static int32 GetNavigationAffectorCost(AActor* Actor);
};
