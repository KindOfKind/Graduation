// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UtilsGridTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GridUtilsFunctionLibrary.generated.h"

struct FGridSizes;
enum class EDirection : uint8;
struct FGridBounds;
struct FGridCellPosition;
/**
 * 
 */
UCLASS()
class CCSUTILS_API UGridUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static FGridCellPosition DirectionToCellPosition(const EDirection& Direction);

	static FGridCellPosition GetGridCellPositionAtLocation(const FVector& Location, const float& CellSize);
	static FVector GetGridCellLocationAtPosition(const FGridCellPosition& CellPosition, const float& CellSize);
	static void ConvertCellPositionsToLocations(TArray<FVector>& OutLocations, const TArray<FGridCellPosition>& CellPositions, const float& CellSize);
	
	static void GetCellsInRadius(TArray<FGridCellPosition>& OutCells, const float& CellSize, const FVector& Location, const float& Radius);
	static void GetGridCellsInVectorBounds(TArray<FGridCellPosition>& OutCells, const float& CellSize, const FVector& MinLocation, const FVector& MaxLocation);
	static void GetGridCellsInBounds(TArray<FGridCellPosition>& OutCells, const FGridBounds& Bounds);

	static void GetAdjacentCells4(TArray<FGridCellPosition>& OutNeighbours, FGridCellPosition CellPosition);

	static void ForEachGridCell(const FGridSizes& GridSizes, const TFunction<void(const FGridCellPosition&)>& Callback);
	static void ForEachGridCell(const FGridBounds& GridBounds, const TFunction<void(const FGridCellPosition&)>& Callback);
	static void ForEachGridCellOnPerimeter(const FGridCellPosition& CenterCell, int32 Offset, const TFunction<void(const FGridCellPosition&)>& Callback);
};
