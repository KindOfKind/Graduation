// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Flowfield/GridTypes.h"
#include "UObject/Object.h"
#include "GridsFunctionsLibrary.generated.h"


UCLASS()
class NAVIGATION_API UGridsFunctionsLibrary : public UObject
{
	GENERATED_BODY()

public:

	inline static bool IsDirectionDiagonal(EDirection& Direction);
	static void GetGridCellValidNeighbours4(TArray<FGridCellPosition>& OutNeighbours, const FCostsGrid& CostsGrid, FGridCellPosition CellPosition);
	static void GetGridCellValidNeighbours8(TArray<FGridCellPosition>& OutNeighbours, const FCostsGrid& CostsGrid, FGridCellPosition CellPosition);

	static void GetGridCellClosestNeighbour8(float const*& OutClosestCellValue, EDirection& OutDirectionToClosestCell, const FIntegrationGrid& IntegrationGrid,
	                                     FGridCellPosition CellPosition);

	static FVector DirectionToVector(const EDirection& Direction);
	static FGridCellPosition DirectionToCellPosition(const EDirection& Direction);

	static void GetGridAreaBounds(FGridBounds& OutBounds, const FVector& BoundsCenter, const float& Radius, const float& CellSize);
	static void GetGridAreaBounds(FGridBounds& OutBounds, const FVector& BottomLeftLocation, const FGridSizes& GridSizes, const float& CellSize);
	// Returns grid size in centimeters
	static FVector2D CalculateGridSize(const FGridSizes& GridSizes, const float& CellSize);

	static void ForEachCostsGridCell(const FCostsGrid& CostsGrid, const TFunction<void(const FGridCellPosition&, const FCostsGridCell&)>& Callback);
	static void ForEachDirectionsGridCell(const FDirectionsGrid& DirectionsGrid, const TFunction<void(const FGridCellPosition&, const FDirectionsGridCell&)>& Callback);
};
