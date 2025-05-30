// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/ClustersHashGrid.h"

#include "Flowfield/Flowfield.h"
#include "Grids/DebugBPFunctionLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"

UClustersHashGrid::UClustersHashGrid()
{
	CellSize = 100;
	ExpansionQueues.AddDefaulted(2);
	CurrentExpansionQueueIndex = 0;
	Flowfield = nullptr;
}

void UClustersHashGrid::Initialize(const AFlowfield* InFlowfield)
{
	Flowfield = InFlowfield;
}

FClusterCellData* UClustersHashGrid::GetClusterDataInCell(const FGridCellPosition& CellPosition)
{
	return ClustersInCells.Find(CellPosition);
}

FClusterCellData* UClustersHashGrid::GetClusterDataAtLocation(const FVector& Location)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	return GetClusterDataInCell(CellPosition);
}

FClusterCellData& UClustersHashGrid::AddClusterDataInCell(const FGridCellPosition& CellPosition, const FClusterCellData& ClusterData)
{
	if (!ClustersInCells.Contains(CellPosition))
	{
		CacheCellByClusterID(ClusterData.ClusterID, CellPosition);
	}
	ExpansionQueues[CurrentExpansionQueueIndex].Enqueue(CellPosition);
	
	return ClustersInCells.Add(CellPosition, ClusterData);
}

FClusterCellData& UClustersHashGrid::AddClusterDataAtLocation(const FVector& Location, const FClusterCellData& ClusterData, FGridCellPosition* OutCellPosition)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	if (OutCellPosition)
	{
		*OutCellPosition = CellPosition;
	}
	return AddClusterDataInCell(CellPosition, ClusterData);
}

void UClustersHashGrid::SetClusterTypeInCell(const FGridCellPosition& CellPosition, FClusterType ClusterType)
{
	ClustersInCells.FindOrAdd(CellPosition).ClusterType = ClusterType;
}

void UClustersHashGrid::SetClusterTypeAtLocation(const FVector& Location, FClusterType ClusterType)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	SetClusterTypeInCell(CellPosition, ClusterType);
}

void UClustersHashGrid::SetClusterTypeForID(const FClusterID& ClusterID, const FClusterType& ClusterType)
{
	ClusterIdToType.Add(ClusterID, ClusterType);

	TArray<FGridCellPosition> ClusterCells;
	GetCellsFromClusterWithID(ClusterCells, ClusterID);
	for (const FGridCellPosition& Cell : ClusterCells)
	{
		ClustersInCells[Cell].ClusterType = ClusterType;
	}
}

void UClustersHashGrid::ClearAllData()
{
	ClustersInCells.Empty();
	CellsCachedByClusterID.Empty();
	ClusterIDs.Empty();
	ExpansionQueues.Empty();
}

void UClustersHashGrid::CacheCellByClusterID(FClusterID ClusterID, const FGridCellPosition& CellPosition)
{
	CellsCachedByClusterID.Add(ClusterID, CellPosition);
	ClusterIDs.Add(ClusterID);
}

int32 UClustersHashGrid::GetNextExpansionQueueIndex()
{
	return (CurrentExpansionQueueIndex == 0 ? 1 : 0);
}

void UClustersHashGrid::ClearCacheForCluster(FClusterID ClusterID)
{
	CellsCachedByClusterID.Remove(ClusterID);
}

void UClustersHashGrid::ExpandClusterIds()
{
	int32 IterationsCounter = 0;
	FGridCellPosition CellPosition;
	while (!ExpansionQueues[0].IsEmpty() || !ExpansionQueues[1].IsEmpty())
	{
		int32 NextQueueIndex = GetNextExpansionQueueIndex();
		
		while (ExpansionQueues[CurrentExpansionQueueIndex].Dequeue(CellPosition))
		{
			ExpandCell(CellPosition, NextQueueIndex);
		}

		CurrentExpansionQueueIndex = NextQueueIndex;

		IterationsCounter += 1;
		if (IterationsCounter >= MaxExpansionIterations)
		{
			break;
		}
	}

	FillEmptyCluster();
}

void UClustersHashGrid::ExpandCell(const FGridCellPosition& CellPosition, const int32 QueueIndex)
{
	TArray<FGridCellPosition> AdjacentCells;
	UGridUtilsFunctionLibrary::GetAdjacentCells4(AdjacentCells, CellPosition);

	for (FGridCellPosition AdjacentCell : AdjacentCells)
	{
		// Continue if the Cell already has a cluster assigned or if the Cell is out of the map borders (defined by flowfield grid borders).
		if (ClustersInCells.Contains(AdjacentCell) || !Flowfield->DoesContainCell(AdjacentCell))
		{
			continue;
		}
		
		FClusterCellData ClusterData = ClustersInCells[CellPosition];
		ClustersInCells.Add(AdjacentCell, ClusterData);
		CellsCachedByClusterID.Add(ClusterData.ClusterID, AdjacentCell);
		ExpansionQueues[QueueIndex].Enqueue(AdjacentCell);
	}
}

void UClustersHashGrid::FillEmptyCluster()
{
	// ToDo: put these constants somewhere else.
	constexpr FClusterID EmptyClusterID = 100;
	constexpr int32 EmptyClusterType = 0;
	FClusterCellData EmptyClusterCellData;
	EmptyClusterCellData.ClusterID = EmptyClusterID;
	EmptyClusterCellData.ClusterType = EmptyClusterType;
	
	FGridBounds FlowfieldBounds;
	Flowfield->GetGridBounds(FlowfieldBounds);
	UGridUtilsFunctionLibrary::ForEachGridCell(FlowfieldBounds, [this, &EmptyClusterCellData](const FGridCellPosition& CellPosition)
	{
		if (ClustersInCells.Contains(CellPosition))
		{
			return;
		}
		ClustersInCells.Add(CellPosition, EmptyClusterCellData);
	});
}


void UClustersHashGrid::DebugDrawClusterIDs(UWorld* WorldContext)
{
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] Invalid World Context!"), __FUNCTION__);
		return;
	}
	
	TMap<FGridCellPosition, FClusterCellData>& Cells = GetClustersInCells();
	for (auto& [CellPosition, CellData] : Cells)
	{
		FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(CellData.ClusterID);
		const FVector CellLocation = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, CellSize);
		DrawDebugLine(WorldContext, CellLocation, CellLocation + FVector::UpVector * 20.f, DebugColor, true, -1.f, 0, 7.f);
	}
}

void UClustersHashGrid::DebugDrawClusterTypes(UWorld* WorldContext)
{
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs] Invalid World Context!"), __FUNCTION__);
		return;
	}
	
	TMap<FGridCellPosition, FClusterCellData>& Cells = GetClustersInCells();
	for (auto& [CellPosition, CellData] : Cells)
	{
		FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(CellData.ClusterType);
		const FVector CellLocation = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, CellSize);
		DrawDebugLine(WorldContext, CellLocation, CellLocation + FVector::UpVector * 20.f, DebugColor, true, -1, 0, 7.f);
	}
}
