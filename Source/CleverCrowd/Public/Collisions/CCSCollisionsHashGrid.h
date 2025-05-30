// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grids/UtilsGridTypes.h"
#include "UObject/Object.h"
#include "CCSCollisionsHashGrid.generated.h"


USTRUCT()
struct CLEVERCROWD_API FCCSCollisionsHashGrid
{
	GENERATED_BODY()

public:
	int32 DenseCellCollisionsCountThreshold = 30;
	int32 MaxCollisionsCount = 40;
	int32 MinDenseAreaSize = 80;	// @note In cells number, not cm.
	int32 MaxDenseAreaSize = 600;
	
	float DecrementCollisionsCountRate = 0.5f;
	int32 DecrementCollisionsCountMag = 2;
	FTimerHandle DecrementCollisionsCountTh;	// Timer has to be Set from outside of this struct and has to call DecrementCollisionsCountAtAllCells()

protected:
	TArray<FGridBounds> DenseAreasCached;	// Dense cells areas that were found during the last search

private:
	int32 CellSize = 100;
	TMap<FGridCellPosition, int32> CollisionsInCells;
	
	FCriticalSection DataLock;

	// DENSE CROWD AREAS SEARCH ------
	
	TSet<FGridCellPosition> DenseCells;
	TSet<FGridCellPosition> VisitedDenseCells;

public:
	FCCSCollisionsHashGrid();

	void operator= (const FCCSCollisionsHashGrid& Other)
	{
		DenseAreasCached  = Other.DenseAreasCached;
		CellSize          = Other.CellSize;
		CollisionsInCells = Other.CollisionsInCells;
		DenseCells        = Other.DenseCells;
		VisitedDenseCells = Other.VisitedDenseCells;
	}
	
public:
	// COMMON ------

	void SearchForDenseAreas(TArray<FGridBounds>& OutDenseAreas);
	TOptional<FGridBounds> SearchForDenseAreaFromCell(const FGridCellPosition& InitCellPosition);

	// COLLISIONS CELLS ------

	TMap<FGridCellPosition, int32>& GetCollisionsInCells() { return CollisionsInCells; };

	int32 GetCellSize() const { return CellSize; }
	int32 GetCollisionsCountAtLocation(const FVector& Location);
	int32 GetCollisionsCountAtCell(const FGridCellPosition& CellPosition);
	void AddCollisionsCountAtLocation(const FVector& Location, const int32 AdditiveCollisions);
	void AddCollisionsCountAtCell(const FGridCellPosition& CellPosition, const int32 AdditiveCollisions);	// Can be used to increase and decrease collision counters

	void DecrementCollisionsCountAtAllCells();

	void CopyMainData(FCCSCollisionsHashGrid& OutCopiedHashGrid) const
	{
		OutCopiedHashGrid.DenseAreasCached  = DenseAreasCached;
		OutCopiedHashGrid.CellSize          = CellSize;
		OutCopiedHashGrid.CollisionsInCells = CollisionsInCells;
	}

	// DEBUG ------

	void DrawDebugDenseAreas(const UWorld* World, const float LifeTime = 5.f, const float Thickness = 4.f);

private:
	void PreSearchForDenseAreas();

	void AddCollisionsCountAtCellNonSync(const FGridCellPosition& CellPosition, const int32 AdditiveCollisions);
};
