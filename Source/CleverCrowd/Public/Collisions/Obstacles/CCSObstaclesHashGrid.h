// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CCSObstaclesTypes.h"
#include "Grids/UtilsGridTypes.h"
#include "UObject/Object.h"
#include "CCSObstaclesHashGrid.generated.h"

/**
 * 
 */
UCLASS()
class CLEVERCROWD_API UCCSObstaclesHashGrid : public UObject
{
	GENERATED_BODY()

private:

	int32 CellSize;
	TMap<FGridCellPosition, TArray<FCCSObstacleEdge>> ObstacleEdgesInCells;

public:

	UCCSObstaclesHashGrid();

public:

	int32 GetCellSize() const { return CellSize; }
	
	void GetObstacleEdgesAtLocation(TArray<FCCSObstacleEdge>& OutEdges, const FVector& Location, const float& Radius);
	void AddObstacleEdge(const FCCSObstacleEdge& Edge);
	void AddObstacleEdges(const TArray<FCCSObstacleEdge>& Edges);

	void GetCellsWithObstacles(TArray<FGridCellPosition>& OutCells);
	void GetCellsWithObstaclesInBounds(TArray<FGridCellPosition>& OutCells, const FGridBounds& Bounds);

	// void AddObstacleRect(const FCCSObstacleRectangle& ObstacleRect);	// Make it call AddObstacleEdge for all 4 rectangle edges

private:

	void FetchObstacleEdgesFromCell(TArray<FCCSObstacleEdge>& OutEdges, const FGridCellPosition& CellPosition);
	void AddObstacleEdgeInCell(const FGridCellPosition& CellPosition, const FCCSObstacleEdge& Edge);
};
