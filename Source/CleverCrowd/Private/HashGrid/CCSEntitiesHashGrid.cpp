// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGrid/CCSEntitiesHashGrid.h"

#include "MassEntityManager.h"
#include "Grids/GridUtilsFunctionLibrary.h"

UCCSEntitiesHashGrid::UCCSEntitiesHashGrid()
{
	CellSize = 100;
}

void UCCSEntitiesHashGrid::ClearData()
{
	EntitiesInCells.Empty();
}

void UCCSEntitiesHashGrid::FetchEntitiesFromCell(TArray<FMassEntityHandle>& OutEntities, const FGridCellPosition& CellPosition)
{
	if (const TArray<FMassEntityHandle>* Entities = EntitiesInCells.Find(CellPosition))
	{
		OutEntities.Append(*Entities);
	}
}

void UCCSEntitiesHashGrid::GetEntitiesInCell(TArray<FMassEntityHandle>& OutEntities, const FGridCellPosition& CellPosition)
{
	if (const TArray<FMassEntityHandle>* Entities = EntitiesInCells.Find(CellPosition))
	{
		OutEntities = *Entities;
	}
}

void UCCSEntitiesHashGrid::GetEntitiesAtLocation(TArray<FMassEntityHandle>& OutEntities, const FVector& Location)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	GetEntitiesInCell(OutEntities, CellPosition);
}

void UCCSEntitiesHashGrid::AddEntityInCell(const FGridCellPosition& CellPosition, const FMassEntityHandle& Entity)
{
	EntitiesInCells.FindOrAdd(CellPosition).Add(Entity);
}

void UCCSEntitiesHashGrid::AddEntityAtLocation(const FVector& Location, const FMassEntityHandle& Entity, const float& EntityRadius)
{
	TArray<FGridCellPosition> AffectedCells;
	UGridUtilsFunctionLibrary::GetCellsInRadius(AffectedCells, CellSize, Location, EntityRadius);
	for (const FGridCellPosition& Cell : AffectedCells)
	{
		AddEntityInCell(Cell, Entity);
	}
}

void UCCSEntitiesHashGrid::GetEntitiesInBounds(TArray<FMassEntityHandle>& OutEntities, const FGridBounds& Bounds)
{
	OutEntities.Empty();
	
	TArray<FGridCellPosition> Cells;
	UGridUtilsFunctionLibrary::GetGridCellsInBounds(Cells, Bounds);
	for (const FGridCellPosition& Cell : Cells)
	{
		FetchEntitiesFromCell(OutEntities, Cell);
	}
}

void UCCSEntitiesHashGrid::ForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&, FMassEntityManager&)>& Callback)
{
	for (auto& [CellPosition, Entities] : EntitiesInCells)
	{
		Callback(CellPosition, *EntityManager);
	}
}

void UCCSEntitiesHashGrid::ForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&, const TArray<FMassEntityHandle>&, FMassEntityManager&)>& Callback)
{
	for (auto& [CellPosition, Entities] : EntitiesInCells)
	{
		Callback(CellPosition, Entities, *EntityManager);
	}
}

void UCCSEntitiesHashGrid::ParallelForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&)>& Callback)
{
	DataLock.Lock();
	TArray<FGridCellPosition> CellsWithEntities;
	EntitiesInCells.GetKeys(CellsWithEntities);
	
	ParallelFor(CellsWithEntities.Num(), [this, &Callback, &CellsWithEntities](const int32 Index)
	{
		Callback(CellsWithEntities[Index]);
	});
	DataLock.Unlock();
}
