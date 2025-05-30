// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridTypes.h"
#include "GameFramework/Actor.h"
#include "Templates/SharedPointer.h"
#include "Flowfield.generated.h"

class AGoalPoint;

UCLASS(Blueprintable)
class NAVIGATION_API AFlowfield : public AActor
{
	GENERATED_BODY()

	friend class UFlowfieldCalculatorComponent;

	TObjectPtr<UFlowfieldCalculatorComponent> FlowfieldCalculatorComponent;

public:
	// GRID CONFIG ------

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FFlowfieldGridSettings GridSettings;

	// GRIDS ------

	TSharedPtr<FCostsGrid> CostsGrid;
	TArray<TSharedPtr<FDirectionsGrid>> DirectionsGrids;	// If there are two different goal points agents may be moving to, the array will contain two grids
	TArray<AGoalPoint*> GoalPoints;	// Goal points to which Directions are calculated. GoalPoints array matches DirectionsGrids array.

	// STATUS ------
	
	bool bCostsGridPendingRecalculation;
	TArray<int32> DirectionsGridPendingRecalculation; // Indices of all Directions Grids that require recalculation

public:
	AFlowfield();

protected:
	virtual void BeginPlay() override;

public:
	void Initialize();

	void RecalculateAllGrids();
	// Initialized costs in all cells of the grid by "scanning" the whole map.
	void RecalculateCostsGrid();

	void CalculateDirectionsGridToGoalPoint(TSharedPtr<FDirectionsGrid> DirectionsGrid, AGoalPoint* GoalPoint);
	// Finds all Goal Points on the map and calculates Directions Grids to them.
	void RecalculateDirectionsGrids();
	// Destroys old Direction Grids and calculates new ones to the Goal Points.
	void RecalculateDirectionsGrids(TArray<AGoalPoint*>& InGoalPoints);

	UFUNCTION(BlueprintCallable)
	FVector GetCenter();
	void GetGridBounds(FGridBounds& OutBounds) const;
	UFUNCTION(BlueprintCallable)
	FVector GetDirectionAtLocation(const FVector& Location, int32 DirectionsGridIndex, bool bUseDetour = true);
	FVector GetDirectionAtCell(const FGridCellPosition& CellPosition, int32 DirectionsGridIndex, bool bUseDetour = true);
	AGoalPoint* GetGoalPoint(int32 DirectionGridIndex);
	bool DoesContainCell(const FGridCellPosition& CellPosition) const;
};
