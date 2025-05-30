// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/MapAnalyzerSubsystem.h"

#include "CrowdEvaluationHashGrid.h"
#include "GameEvaluatorSubsystem.h"
#include "Clustering/KMeans.h"
#include "Collisions/Obstacles/CCSObstaclesHashGrid.h"
#include "Common/CommonTypes.h"
#include "Global/CrowdNavigationSubsystem.h"
#include "Grids/DebugBPFunctionLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Management/CCSCollisionsSubsystem.h"
#include "MapAnalyzer/ClustersHashGrid.h"
#include "MapAnalyzer/ClusterDefiners/MapClusterDefinerBase.h"
#include "MapAnalyzer/MapArea.h"

void UMapAnalyzerSubsystem::PostInitialize()
{
}

void UMapAnalyzerSubsystem::InitializeOuter()
{
	CrowdNavigationSubsystem = GetWorld()->GetSubsystem<UCrowdNavigationSubsystem>();
	CollisionsSubsystem      = GetWorld()->GetSubsystem<UCCSCollisionsSubsystem>();
	CrowdEvaluationGrid      = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>()->GetEvaluationHashGrid();
}

void UMapAnalyzerSubsystem::SetMapClusterizationRules(const FClusterizationRules& InClusterizationRules)
{
	ClusterizationRules = InClusterizationRules;
}

void UMapAnalyzerSubsystem::DefineClustersOnMap()
{
	// Find locations of all hash grid cells that contain an obstacle ---
	TArray<FGridCellPosition> CellsWithObstacles;
	UCCSObstaclesHashGrid* ObstaclesHashGrid = CollisionsSubsystem->GetObstaclesHashGrid();
	check(ObstaclesHashGrid);
	ObstaclesHashGrid->GetCellsWithObstacles(CellsWithObstacles);

	// Convert CellPositions to Locations ---
	TArray<FVector> PointsLocations;
	UGridUtilsFunctionLibrary::ConvertCellPositionsToLocations(PointsLocations, CellsWithObstacles, ObstaclesHashGrid->GetCellSize());

	// Group locations into clusters ---
	TArray<TArray<int32>> ClusteredPoints;	// [ClusterID][PointIndex]
	UE::Geometry::FClusterKMeans Clustering;
	Clustering.MaxIterations = 500;
	GroupPointsIntoClusters(ClusteredPoints, Clustering, ClusterizationRules, PointsLocations);

	// Assign ClusterIDs to ClustersHashGrid cells
	for (int32 ClusterID = 0; ClusterID < ClusteredPoints.Num(); ClusterID++)
	{
		for (const int32 PointIdx : ClusteredPoints[ClusterID])
		{
			FVector PointLoc = PointsLocations[PointIdx];
			FClusterCellData ClusterCellData{ClusterID, -1};
			GetClustersHashGrid()->AddClusterDataAtLocation(PointLoc, ClusterCellData);
		}
	}

	// "Expand" clusters so that each cell has a cluster type assigned
	GetClustersHashGrid()->ExpandClusterIds();

	if (ClusterizationRules.bDrawClusterIDsDebug)
	{
		GetClustersHashGrid()->DebugDrawClusterIDs(GetWorld());
	}

	// Analyze each cluster of cells with Clusters Definer and assign a determined ClusterType to each
	ClusterizationRules.ClusterDefiner->Initialize(this);
	const TSet<UClustersHashGrid::FClusterID>& ClustersIDs = GetClustersHashGrid()->GetClusterIDs();
	for (const UClustersHashGrid::FClusterID ClusterID : ClustersIDs)
	{
		CCSMapClustering::FClusterType ClusterType = ClusterizationRules.ClusterDefiner->DefineClusterType(ClusterID, CellsWithObstacles);
		GetClustersHashGrid()->SetClusterTypeForID(ClusterID, ClusterType);
	}

	if (ClusterizationRules.bDrawClusterTypesDebug)
	{
		GetClustersHashGrid()->DebugDrawClusterTypes(GetWorld());
	}
}

UClustersHashGrid* UMapAnalyzerSubsystem::GetClustersHashGrid()
{
	if (!ClustersHashGrid)
	{
		ClustersHashGrid = NewObject<UClustersHashGrid>();
		ClustersHashGrid->Initialize(CrowdNavigationSubsystem->GetFlowfield());
	}

	return ClustersHashGrid;
}

UCCSObstaclesHashGrid* UMapAnalyzerSubsystem::GetObstaclesHashGrid()
{
	return CollisionsSubsystem->GetObstaclesHashGrid();
}

void UMapAnalyzerSubsystem::DefineMapAreasOnMap()
{
	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapArea::StaticClass(), SpawnerActors);
	for (int32 i = 0; i < SpawnerActors.Num(); i++)
	{
		DefineMapAreaInBounds(Cast<AMapArea>(SpawnerActors[i])->GetBounds());
	}

	for (auto& [AreaId, OtherAreaData] : MapAreasDataConfig)
	{
		if (AreaId > MaxMapAreaTypeId)
		{
			MaxMapAreaTypeId = AreaId;
		}
	}
}

void UMapAnalyzerSubsystem::DefineMapAreaInBounds(const FGridBounds& Bounds)
{
	// Get statistics of the area in the specified bounds
	FMapAreaData AreaData;
	EvaluateMapAreaDataInBounds(AreaData, Bounds);

	// Find the most similar area type.
	// @note We assume that AreaData is normalized
	float MaxSimilarity     = 0.f;
	int32 MostSimilarAreaId = INDEX_NONE;

	for (auto& [AreaId, OtherAreaData] : MapAreasDataConfig)
	{
		float SimilaritiesSum = 0.f;
		for (int32 ParamIdx = 0; ParamIdx < FMapAreaData::FloatParamsNum; ParamIdx++)
		{
			SimilaritiesSum += 1.f - abs(AreaData.GetParameterValue(ParamIdx) - OtherAreaData.GetParameterValue(ParamIdx));
		}
		float Similarity = SimilaritiesSum / FMapAreaData::FloatParamsNum; // Average of similarities of all params

		if (MaxSimilarity < Similarity)
		{
			MaxSimilarity     = Similarity;
			MostSimilarAreaId = AreaId;
		}
	}

	// Cache in hash grid
	UGridUtilsFunctionLibrary::ForEachGridCell(Bounds, [this, MostSimilarAreaId](const FGridCellPosition& CellPosition)
	{
		CrowdEvaluationGrid->SetAreaIdInCell(CellPosition, MostSimilarAreaId);
	});

	constexpr bool bDrawDebug = true;
	if (bDrawDebug)
	{
		const int32 CellSize = 100;
		FVector Extent       = Bounds.GetExtent(static_cast<float>(CellSize));
		Extent.Z += 500.f;
		FVector Center    = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(Bounds.GetCenterCell(), CellSize);
		FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(MostSimilarAreaId);
		DrawDebugBox(GetWorld(), Center, Extent, DebugColor, false, 10.f, 0, 20.f);
	}
}

void UMapAnalyzerSubsystem::EvaluateMapAreaDataInBounds(FMapAreaData& OutAreaData, const FGridBounds& Bounds)
{
	// ToDo: implement (iterate through all cells and collect info)

	UCCSObstaclesHashGrid* ObstaclesHashGrid = CollisionsSubsystem->GetObstaclesHashGrid();
	check(ObstaclesHashGrid);

	TArray<FGridCellPosition> CellsWithObstacles;
	ObstaclesHashGrid->GetCellsWithObstaclesInBounds(CellsWithObstacles, Bounds);

	const int32 AllCellsNum = Bounds.GetArea();
	const int32 ObstacleCellsNum = CellsWithObstacles.Num();

	// ObstacleCellsFraction param ------
	OutAreaData.ObstacleCellsFraction = static_cast<float>(ObstacleCellsNum) / AllCellsNum;

	// (Average) ObstaclesIslandSize param ------
	TMap<FGridCellPosition, int32> ObstacleCellsByIslands;	// [ObstacleCell][IslandIndex]
	TArray<FGridCellPosition> CellsWithObstaclesTemp = CellsWithObstacles;
	TArray<float> IslandsSizes;
	int32 IslandIndex = 0;
	while (CellsWithObstaclesTemp.IsEmpty() == false)
	{
		int32 ObstacleCellsCounter = 0;
		TQueue<FGridCellPosition> CellsQueue;
		TSet<FGridCellPosition> VisitedCells;
		CellsQueue.Enqueue(CellsWithObstaclesTemp[0]);
		VisitedCells.Add(CellsWithObstaclesTemp[0]);

		FGridCellPosition Cell;
		while (CellsQueue.Dequeue(Cell))
		{
			ObstacleCellsCounter += 1;
			CellsWithObstaclesTemp.Remove(Cell);
			ObstacleCellsByIslands.Add(Cell, IslandIndex);

			TArray<FGridCellPosition> AdjacentCells;
			UGridUtilsFunctionLibrary::GetAdjacentCells4(AdjacentCells, Cell);
			for (const FGridCellPosition& AdjacentCell : AdjacentCells)
			{
				if (!CellsWithObstaclesTemp.Contains(AdjacentCell) || VisitedCells.Contains(AdjacentCell))
				{
					continue;
				}
				VisitedCells.Add(AdjacentCell);
				CellsQueue.Enqueue(AdjacentCell);
			}
		}
		
		IslandsSizes.Add(static_cast<float>(ObstacleCellsCounter) / AllCellsNum);
		IslandIndex += 1;
	}

	float IslandSizesSum = 0;
	for (float IslandSize : IslandsSizes)
	{
		IslandSizesSum += IslandSize;
	}
	OutAreaData.ObstaclesIslandSize = IslandSizesSum / IslandsSizes.Num();

	// IslandsProximity param ------
	FAggregatedValueFloat NormalizedAggregatedProximity;
	for (const FGridCellPosition& Cell : CellsWithObstacles)
	{
		const int32 CurrentIslandIndex = *ObstacleCellsByIslands.Find(Cell);
		int32 OtherIslandsNearbyCells = 0;
		auto CountNearbyCellsLambda = [this, CurrentIslandIndex, &OtherIslandsNearbyCells, &ObstacleCellsByIslands](const FGridCellPosition& OtherCell)
		{
			const int32* OtherIslandIndex = ObstacleCellsByIslands.Find(OtherCell);
			if (OtherIslandIndex != nullptr && *OtherIslandIndex != CurrentIslandIndex)
			{
				OtherIslandsNearbyCells += 1;
			}
		};
		UGridUtilsFunctionLibrary::ForEachGridCellOnPerimeter(Cell, 2, CountNearbyCellsLambda);
		UGridUtilsFunctionLibrary::ForEachGridCellOnPerimeter(Cell, 3, CountNearbyCellsLambda);
		NormalizedAggregatedProximity.AddValue(FMath::Clamp(static_cast<float>(OtherIslandsNearbyCells) / 4.f, 0.f, 1.f));
	}

	OutAreaData.IslandsProximity = NormalizedAggregatedProximity.GetMean();

	// (Average) CornersCountInIsland param ------
	// ToDo: implement
	CellsWithObstaclesTemp = CellsWithObstacles;

	
	// UGridUtilsFunctionLibrary::ForEachGridCell(Bounds, [this](const FGridCellPosition& CellPosition)
	// {
	// 	
	// });
}


void UMapAnalyzerSubsystem::GroupPointsIntoClusters(TArray<TArray<int32>>& OutClusteredPoints, UE::Geometry::FClusterKMeans& Clustering,
                                                    const FClusterizationRules& InClusterizationRules, const TArray<FVector>& PointsLocations)
{
	const int32 ClustersNum      = InClusterizationRules.ClustersNum;
	const bool bDrawDebug = InClusterizationRules.bDrawInitialPointsDebug;
	
	TArray<FVector> InitialCentroids;
	const TArrayView<const FVector> PointsLocationsView = PointsLocations;
	Clustering.GetUniformSpacedInitialCenters(PointsLocationsView, ClustersNum, InitialCentroids);

	const TArrayView<const FVector> InitialCentroidsView = InitialCentroids;
	Clustering.ComputeClusters(PointsLocationsView, ClustersNum, InitialCentroidsView);
	Clustering.GetClusters(OutClusteredPoints);

	if (bDrawDebug)
	{
		for (int32 ClusterID = 0; ClusterID < OutClusteredPoints.Num(); ClusterID++)
		{
			FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(ClusterID);
			for (const int32 PointIdx : OutClusteredPoints[ClusterID])
			{
				FVector PointLoc = PointsLocations[PointIdx];
				DrawDebugLine(GetWorld(), PointLoc, PointLoc + FVector::UpVector * 1000.f, DebugColor, true, -1, 0, 7.f);
			}
		}
	}
}
