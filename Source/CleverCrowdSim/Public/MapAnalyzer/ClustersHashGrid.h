// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapClusterizationTypes.h"
#include "Grids/UtilsGridTypes.h"
#include "UObject/Object.h"
#include "ClustersHashGrid.generated.h"


class AFlowfield;

UCLASS()
class CLEVERCROWDSIM_API UClustersHashGrid : public UObject
{
	GENERATED_BODY()

	friend class UMapAnalyzerSubsystem;

public:
	typedef int32 FClusterType;
	typedef int32 FClusterID;

	const int32 MaxExpansionIterations = 10;

private:
	int32 CellSize;
	TMap<FGridCellPosition, FClusterCellData> ClustersInCells;
	TMultiMap<FClusterID, FGridCellPosition> CellsCachedByClusterID; // Used to effectively analyze all cluster cells
	TSet<FClusterID> ClusterIDs;                                     // IDs of all non-empty clusters
	TMap<FClusterID, FClusterType> ClusterIdToType;                  // Describes ClusterType for each ClusterID

	// Fields used to evenly distribute ClusterIDs among cells (by expanding data from queued cells)
	int32 CurrentExpansionQueueIndex;
	TArray<TQueue<FGridCellPosition>> ExpansionQueues;

	UPROPERTY()
	const AFlowfield* Flowfield;

public:
	UClustersHashGrid();

public:
	FClusterCellData* GetClusterDataInCell(const FGridCellPosition& CellPosition);
	FClusterCellData* GetClusterDataAtLocation(const FVector& Location);
	FClusterCellData& AddClusterDataInCell(const FGridCellPosition& CellPosition, const FClusterCellData& ClusterData);
	FClusterCellData& AddClusterDataAtLocation(const FVector& Location, const FClusterCellData& ClusterData, FGridCellPosition* OutCellPosition = nullptr);
	void SetClusterTypeInCell(const FGridCellPosition& CellPosition, FClusterType ClusterType);
	void SetClusterTypeAtLocation(const FVector& Location, FClusterType ClusterType);
	void SetClusterTypeForID(const FClusterID& ClusterID, const FClusterType& ClusterType);
	void ClearAllData();
	void ClearCacheForCluster(FClusterID ClusterID);

	// "Expand" clusters so that each cell has a cluster type assigned.
	// Cells that are far from any cell initialized with ClusterID will be of "empty cluster" type. 
	void ExpandClusterIds();

	TMap<FGridCellPosition, FClusterCellData>& GetClustersInCells() { return ClustersInCells; }
	TSet<FClusterID>& GetClusterIDs() { return ClusterIDs; };
	void GetCellsFromClusterWithID(TArray<FGridCellPosition>& OutGridCells, int32 ClusterID) const { return CellsCachedByClusterID.MultiFind(ClusterID, OutGridCells); };

	void DebugDrawClusterIDs(UWorld* WorldContext);
	void DebugDrawClusterTypes(UWorld* WorldContext);
	
private:
	void Initialize(const AFlowfield* InFlowfield);
	
	void CacheCellByClusterID(FClusterID ClusterID, const FGridCellPosition& CellPosition);
	void ExpandCell(const FGridCellPosition& CellPosition, const int32 QueueIndex);
	void FillEmptyCluster();
	int32 GetNextExpansionQueueIndex();
};
