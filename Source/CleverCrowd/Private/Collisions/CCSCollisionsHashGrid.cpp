// Fill out your copyright notice in the Description page of Project Settings.


#include "Collisions/CCSCollisionsHashGrid.h"

#include "Grids/GridUtilsFunctionLibrary.h"

FCCSCollisionsHashGrid::FCCSCollisionsHashGrid()
{
}

void FCCSCollisionsHashGrid::SearchForDenseAreas(TArray<FGridBounds>& OutDenseAreas)
{
	DataLock.Lock();

	PreSearchForDenseAreas();

	for (const FGridCellPosition& DenseCell : DenseCells)
	{
		if (VisitedDenseCells.Contains(DenseCell))
		{
			continue;
		}

		TOptional<FGridBounds> DenseAreaBounds = SearchForDenseAreaFromCell(DenseCell);
		if (!DenseAreaBounds.IsSet() || DenseAreaBounds->GetArea() < MinDenseAreaSize)
		{
			continue;
		}

		OutDenseAreas.Add(DenseAreaBounds.GetValue());
	}

	DenseAreasCached = OutDenseAreas;
	
	DataLock.Unlock();
}

TOptional<FGridBounds> FCCSCollisionsHashGrid::SearchForDenseAreaFromCell(const FGridCellPosition& InitCellPosition)
{
	TQueue<FGridCellPosition> CellsQueue;
	CellsQueue.Enqueue(InitCellPosition);
	VisitedDenseCells.Add(InitCellPosition);
	FGridBounds DenseAreaBounds{InitCellPosition, InitCellPosition};

	FGridCellPosition Cell;
	while (CellsQueue.Dequeue(Cell))
	{
		if (DenseAreaBounds.GetArea() > MaxDenseAreaSize)
		{
			break;
		}
		
		DenseAreaBounds.UpdateToFitCell(Cell);

		TArray<FGridCellPosition> AdjacentCells;
		UGridUtilsFunctionLibrary::GetAdjacentCells4(AdjacentCells, Cell);
		for (const FGridCellPosition& AdjacentCell : AdjacentCells)
		{
			if (!DenseCells.Contains(AdjacentCell) || VisitedDenseCells.Contains(AdjacentCell))
			{
				continue;
			}
			VisitedDenseCells.Add(AdjacentCell);
			CellsQueue.Enqueue(AdjacentCell);
		}
	}

	if (DenseAreaBounds.GetArea() < MinDenseAreaSize)
	{
		return TOptional<FGridBounds>();
	}
	
	return DenseAreaBounds;
}

int32 FCCSCollisionsHashGrid::GetCollisionsCountAtLocation(const FVector& Location)
{
	return GetCollisionsCountAtCell(UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize));
}

int32 FCCSCollisionsHashGrid::GetCollisionsCountAtCell(const FGridCellPosition& CellPosition)
{
	const int32* CollisionsCount = CollisionsInCells.Find(CellPosition);
	if(CollisionsCount)
	{
		return *CollisionsCount;
	}
	return 0;
}

void FCCSCollisionsHashGrid::AddCollisionsCountAtLocation(const FVector& Location, const int32 AdditiveCollisions)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	AddCollisionsCountAtCell(CellPosition, AdditiveCollisions);
}

void FCCSCollisionsHashGrid::AddCollisionsCountAtCell(const FGridCellPosition& CellPosition, const int32 AdditiveCollisions)
{
	if (!DataLock.TryLock())	// This makes this method non-reliable, but more performant
	{
		return;
	}

	AddCollisionsCountAtCellNonSync(CellPosition, AdditiveCollisions);
	
	DataLock.Unlock();
}

void FCCSCollisionsHashGrid::AddCollisionsCountAtCellNonSync(const FGridCellPosition& CellPosition, const int32 AdditiveCollisions)
{
	int32& CollisionsCount = CollisionsInCells.FindOrAdd(CellPosition);
	CollisionsCount = FMath::Clamp(CollisionsCount + AdditiveCollisions, 0, MaxCollisionsCount);

	if (CollisionsCount >= DenseCellCollisionsCountThreshold)
	{
		DenseCells.Add(CellPosition);
	}
	else if (CollisionsCount < DenseCellCollisionsCountThreshold)
	{
		DenseCells.Remove(CellPosition);
	}
}

void FCCSCollisionsHashGrid::DecrementCollisionsCountAtAllCells()
{
	DataLock.Lock();
	for (auto& [CellPosition, Count] : CollisionsInCells)
	{
		AddCollisionsCountAtCellNonSync(CellPosition, -DecrementCollisionsCountMag);
	}
	DataLock.Unlock();
}


void FCCSCollisionsHashGrid::DrawDebugDenseAreas(const UWorld* World, const float LifeTime, const float Thickness)
{
	for (const FGridBounds& DenseArea : DenseAreasCached)
	{
		FVector Extent = DenseArea.GetExtent(static_cast<float>(CellSize));
		Extent.Z += 500.f;
		FVector Center = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(DenseArea.GetCenterCell(), CellSize);
		DrawDebugBox(World, Center, Extent, FColor::Red, false, LifeTime, 0, Thickness);
	}
}


void FCCSCollisionsHashGrid::PreSearchForDenseAreas()
{
	VisitedDenseCells.Reset();
}
