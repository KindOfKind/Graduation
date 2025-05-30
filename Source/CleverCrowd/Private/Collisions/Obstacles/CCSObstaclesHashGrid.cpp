// Fill out your copyright notice in the Description page of Project Settings.


#include "Collisions/Obstacles/CCSObstaclesHashGrid.h"

#include "Grids/GridUtilsFunctionLibrary.h"

UCCSObstaclesHashGrid::UCCSObstaclesHashGrid()
{
	CellSize = 100;
}

void UCCSObstaclesHashGrid::GetObstacleEdgesAtLocation(TArray<FCCSObstacleEdge>& OutEdges, const FVector& Location, const float& Radius)
{
	TArray<FGridCellPosition> AffectedCells;
	UGridUtilsFunctionLibrary::GetCellsInRadius(AffectedCells, CellSize, Location, Radius);
	for (const FGridCellPosition& CellPos : AffectedCells)
	{
		FetchObstacleEdgesFromCell(OutEdges, CellPos);
	}
}

void UCCSObstaclesHashGrid::AddObstacleEdge(const FCCSObstacleEdge& Edge)
{
	FVector MinLoc = FVector::ZeroVector;
	FVector MaxLoc = FVector::ZeroVector;
	MinLoc.X = FMath::Min(Edge.Start.X, Edge.End.X);
	MinLoc.Y = FMath::Min(Edge.Start.Y, Edge.End.Y);
	MaxLoc.X = FMath::Max(Edge.Start.X, Edge.End.X);
	MaxLoc.Y = FMath::Max(Edge.Start.Y, Edge.End.Y);

	TArray<FGridCellPosition> AffectedCells;
	UGridUtilsFunctionLibrary::GetGridCellsInVectorBounds(AffectedCells, CellSize, MinLoc, MaxLoc);

	for (const FGridCellPosition& CellPos : AffectedCells)
	{
		AddObstacleEdgeInCell(CellPos, Edge);
	}
}

void UCCSObstaclesHashGrid::AddObstacleEdges(const TArray<FCCSObstacleEdge>& Edges)
{
	for (const FCCSObstacleEdge& Edge : Edges)
	{
		AddObstacleEdge(Edge);
	}
}

void UCCSObstaclesHashGrid::GetCellsWithObstacles(TArray<FGridCellPosition>& OutCells)
{
	for (auto& [CellPosition, ObstacleEdges] : ObstacleEdgesInCells)
	{
		if (ObstacleEdges.IsEmpty()) continue;
		OutCells.Add(CellPosition);
	}
}

void UCCSObstaclesHashGrid::GetCellsWithObstaclesInBounds(TArray<FGridCellPosition>& OutCells, const FGridBounds& Bounds)
{
	for (auto& [CellPosition, ObstacleEdges] : ObstacleEdgesInCells)
	{
		if (ObstacleEdges.IsEmpty() || !Bounds.IsCellInBounds(CellPosition)) continue;
		OutCells.Add(CellPosition);
	}
}

void UCCSObstaclesHashGrid::FetchObstacleEdgesFromCell(TArray<FCCSObstacleEdge>& OutEdges, const FGridCellPosition& CellPosition)
{
	if (const TArray<FCCSObstacleEdge>* Edges = ObstacleEdgesInCells.Find(CellPosition))
	{
		OutEdges.Append(*Edges);
	}
}

void UCCSObstaclesHashGrid::AddObstacleEdgeInCell(const FGridCellPosition& CellPosition, const FCCSObstacleEdge& Edge)
{
	ObstacleEdgesInCells.FindOrAdd(CellPosition).Add(Edge);
}
