// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Misc/GridsFunctionsLibrary.h"

#include "Grids/GridUtilsFunctionLibrary.h"

bool UGridsFunctionsLibrary::IsDirectionDiagonal(EDirection& Direction)
{
	return (static_cast<uint8>(Direction) % 2 == 1);
}

void UGridsFunctionsLibrary::GetGridCellValidNeighbours4(TArray<FGridCellPosition>& OutNeighbours, const FCostsGrid& CostsGrid, FGridCellPosition CellPosition)
{
	for (EDirection Direction : NavigationGlobals::Directions4)
	{
		const FGridCellPosition NeighbourPosition = DirectionToCellPosition(Direction) + CellPosition;
		if (!CostsGrid.Contains(NeighbourPosition))
		{
			continue;
		}
		
		OutNeighbours.Add(NeighbourPosition);
	}
}

void UGridsFunctionsLibrary::GetGridCellValidNeighbours8(TArray<FGridCellPosition>& OutNeighbours, const FCostsGrid& CostsGrid, FGridCellPosition CellPosition)
{
	for (EDirection Direction : TEnumRange<EDirection>())
	{
		const FGridCellPosition NeighbourPosition = DirectionToCellPosition(Direction) + CellPosition;
		if (!CostsGrid.Contains(NeighbourPosition))
		{
			continue;
		}
		
		OutNeighbours.Add(NeighbourPosition);
	}
}

void UGridsFunctionsLibrary::GetGridCellClosestNeighbour8(float const*& OutClosestCellValue, EDirection& OutDirectionToClosestCell, const FIntegrationGrid& IntegrationGrid,
                                                      FGridCellPosition CellPosition)
{
	int32 MinCost = INT32_MAX;

	for (EDirection Direction : TEnumRange<EDirection>())
	{
		const FGridCellPosition NeighbourPosition = DirectionToCellPosition(Direction) + CellPosition;
		const float* AdjacentCellValue            = IntegrationGrid.Cells.Find(NeighbourPosition);
		if (!AdjacentCellValue)
		{
			continue;
		}

		if (*AdjacentCellValue < MinCost)
		{
			OutDirectionToClosestCell = Direction;
			OutClosestCellValue       = AdjacentCellValue;
			MinCost                   = *AdjacentCellValue;
		}
	}
}

FVector UGridsFunctionsLibrary::DirectionToVector(const EDirection& Direction)
{
	const int32 DirectionIdx = static_cast<int32>(Direction);
	checkf(NavigationGlobals::DirectionVectors.IsValidIndex(DirectionIdx), L"Invalid direction!");
	
	return NavigationGlobals::DirectionVectors[DirectionIdx];
}

FGridCellPosition UGridsFunctionsLibrary::DirectionToCellPosition(const EDirection& Direction)
{
	const int32 DirectionIdx = static_cast<int32>(Direction);
	checkf(NavigationGlobals::DirectionVectors.IsValidIndex(DirectionIdx), L"Invalid direction!");
	const FVector Vector = NavigationGlobals::DirectionVectors[DirectionIdx];
	
	return FGridCellPosition{static_cast<int32>(Vector.X), static_cast<int32>(Vector.Y)};
}

void UGridsFunctionsLibrary::GetGridAreaBounds(FGridBounds& OutBounds, const FVector& BoundsCenter, const float& Radius, const float& CellSize)
{
	const FGridCellPosition CenterCellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(BoundsCenter, CellSize);
	const int32 CellsNumberInRadius            = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(FVector{Radius, 0.f, 0.f}, CellSize).X;
	
	OutBounds.BottomLeftCell = CenterCellPosition - FGridCellPosition{CellsNumberInRadius, CellsNumberInRadius};
	OutBounds.TopRightCell   = CenterCellPosition + FGridCellPosition{CellsNumberInRadius, CellsNumberInRadius};
}

void UGridsFunctionsLibrary::GetGridAreaBounds(FGridBounds& OutBounds, const FVector& BottomLeftLocation, const FGridSizes& GridSizes, const float& CellSize)
{
	const FGridCellPosition BottomLeftPos = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(BottomLeftLocation, CellSize);
	
	OutBounds.BottomLeftCell = BottomLeftPos;
	OutBounds.TopRightCell   = BottomLeftPos + FGridCellPosition{GridSizes.Cols, GridSizes.Rows};
}

FVector2D UGridsFunctionsLibrary::CalculateGridSize(const FGridSizes& GridSizes, const float& CellSize)
{
	return FVector2D{GridSizes.Cols * CellSize, GridSizes.Rows * CellSize};
}

void UGridsFunctionsLibrary::ForEachCostsGridCell(const FCostsGrid& CostsGrid, const TFunction<void(const FGridCellPosition&, const FCostsGridCell&)>& Callback)
{
	for (auto& [CellPosition, Cell] : CostsGrid.Cells)
	{
		Callback(CellPosition, Cell);
	}
}

void UGridsFunctionsLibrary::ForEachDirectionsGridCell(const FDirectionsGrid& DirectionsGrid, const TFunction<void(const FGridCellPosition&, const FDirectionsGridCell&)>& Callback)
{
	for (auto& [CellPosition, Cell] : DirectionsGrid.Cells)
	{
		Callback(CellPosition, Cell);
	}
}
