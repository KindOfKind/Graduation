// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Grids/UtilsGridTypes.h"
#include "UObject/Object.h"
#include "CCSEntitiesHashGrid.generated.h"


class UCCSEntitiesManagerSubsystem;

UCLASS()
class CLEVERCROWD_API UCCSEntitiesHashGrid : public UObject
{
	GENERATED_BODY()

	friend UCCSEntitiesManagerSubsystem;

public:
	FCriticalSection DataLock;

protected:
	FMassEntityManager* EntityManager;
	
	int32 CellSize;
	// ToDo: remove destroyed entities from this container.
	TMap<FGridCellPosition, TArray<FMassEntityHandle>> EntitiesInCells;

public:
	UCCSEntitiesHashGrid();

public:
	// Removes all entities from all grid cells
	void ClearData();
	void FetchEntitiesFromCell(TArray<FMassEntityHandle>& OutEntities, const FGridCellPosition& CellPosition);	// Unlike Get, Fetch will not clear old OutEntities data
	void GetEntitiesInCell(TArray<FMassEntityHandle>& OutEntities, const FGridCellPosition& CellPosition);
	void GetEntitiesAtLocation(TArray<FMassEntityHandle>& OutEntities, const FVector& Location);
	void GetEntitiesInBounds(TArray<FMassEntityHandle>& OutEntities, const FGridBounds& Bounds);
	void AddEntityInCell(const FGridCellPosition& CellPosition, const FMassEntityHandle& Entity);
	void AddEntityAtLocation(const FVector& Location, const FMassEntityHandle& Entity, const float& EntityRadius);

	void ForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&, FMassEntityManager&)>& Callback);
	void ForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&, const TArray<FMassEntityHandle>&, FMassEntityManager&)>& Callback);
	void ParallelForEachNonEmptyCell(const TFunction<void(const FGridCellPosition&)>& Callback);
};
