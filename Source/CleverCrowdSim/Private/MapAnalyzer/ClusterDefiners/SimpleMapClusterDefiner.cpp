// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/ClusterDefiners/SimpleMapClusterDefiner.h"

#include "MapAnalyzer/ClustersHashGrid.h"
#include "MapAnalyzer/MapAnalyzerSubsystem.h"

UMapClusterDefinerBase::FClusterType USimpleMapClusterDefiner::DefineClusterType(const int32 ClusterID, const TArray<FGridCellPosition>& NonEmptyCells)
{
	check(MapAnalyzer);

	TArray<FGridCellPosition> ClusterCellsArray;
	MapAnalyzer->GetClustersHashGrid()->GetCellsFromClusterWithID(ClusterCellsArray, ClusterID);
	TSet<FGridCellPosition> ClusterCells;
	for (const FGridCellPosition& Cell : ClusterCellsArray)
	{
		ClusterCells.Add(Cell);
	}
	
	int32 BlockedCellsCount = 0;
	for (const FGridCellPosition& NonEmptyCell : NonEmptyCells)
	{
		if (ClusterCells.Contains(NonEmptyCell))
		{
			BlockedCellsCount += 1; 
		}
	}

	const float Density = static_cast<float>(BlockedCellsCount) / static_cast<float>(ClusterCells.Num());
	if (Density > 0.2f)
	{
		return CCSMapClustering::ClusterTypes::Dense;
	}
	else
	{
		return CCSMapClustering::ClusterTypes::Spacious;
	}
}
