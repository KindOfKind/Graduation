// Fill out your copyright notice in the Description page of Project Settings.


#include "Grids/GridUtilsFunctionLibrary.h"

#include "Grids/UtilsGridTypes.h"


FGridCellPosition UGridUtilsFunctionLibrary::DirectionToCellPosition(const EDirection& Direction)
{
	const int32 DirectionIdx = static_cast<int32>(Direction);
	checkf(NavigationGlobals::DirectionVectors.IsValidIndex(DirectionIdx), L"Invalid direction!");
	const FVector Vector = NavigationGlobals::DirectionVectors[DirectionIdx];
	
	return FGridCellPosition{static_cast<int32>(Vector.X), static_cast<int32>(Vector.Y)};
}


FGridCellPosition UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(const FVector& Location, const float& CellSize)
{
	FGridCellPosition CellPosition;

	CellPosition.X = FMath::Floor(Location.X / CellSize);
	CellPosition.Y = FMath::Floor(Location.Y / CellSize);

	return CellPosition;
}

FVector UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(const FGridCellPosition& CellPosition, const float& CellSize)
{
	const float OffsetToCenter = CellSize / 2.f;
	return FVector{CellPosition.X * CellSize + OffsetToCenter, CellPosition.Y * CellSize + OffsetToCenter, 0.0f};
}

void UGridUtilsFunctionLibrary::ConvertCellPositionsToLocations(TArray<FVector>& OutLocations, const TArray<FGridCellPosition>& CellPositions, const float& CellSize)
{
	OutLocations.Empty();
	OutLocations.Reserve(CellPositions.Num());
	for (const FGridCellPosition& CellPosition : CellPositions)
	{
		OutLocations.Add(GetGridCellLocationAtPosition(CellPosition, CellSize));
	}
}

void UGridUtilsFunctionLibrary::GetCellsInRadius(TArray<FGridCellPosition>& OutCells, const float& CellSize, const FVector& Location, const float& Radius)
{
	FGridCellPosition BottomLeftCell = GetGridCellPositionAtLocation(Location + FVector{-Radius, -Radius, 0.f}, CellSize); // Upper left corner
	FGridCellPosition TopRightCell   = GetGridCellPositionAtLocation(Location + FVector{Radius, Radius, 0.f}, CellSize);   // Bottom right corner
	FGridBounds BoundsInRadius{BottomLeftCell, TopRightCell};

	UGridUtilsFunctionLibrary::GetGridCellsInBounds(OutCells, BoundsInRadius);
}

void UGridUtilsFunctionLibrary::GetGridCellsInVectorBounds(TArray<FGridCellPosition>& OutCells, const float& CellSize, const FVector& MinLocation, const FVector& MaxLocation)
{
	FGridCellPosition BottomLeftCell = GetGridCellPositionAtLocation(MinLocation, CellSize); // Upper left corner
	FGridCellPosition TopRightCell   = GetGridCellPositionAtLocation(MaxLocation, CellSize); // Bottom right corner
	FGridBounds Bounds{BottomLeftCell, TopRightCell};

	UGridUtilsFunctionLibrary::GetGridCellsInBounds(OutCells, Bounds);
}

void UGridUtilsFunctionLibrary::GetGridCellsInBounds(TArray<FGridCellPosition>& OutCells, const FGridBounds& Bounds)
{
	for (int32 Row = Bounds.BottomLeftCell.Y; Row <= Bounds.TopRightCell.Y; Row++)
	{
		for (int32 Col = Bounds.BottomLeftCell.X; Col <= Bounds.TopRightCell.X; Col++)
		{
			OutCells.Add(FGridCellPosition{Col, Row});
		}
	}
}

void UGridUtilsFunctionLibrary::GetAdjacentCells4(TArray<FGridCellPosition>& OutNeighbours, FGridCellPosition CellPosition)
{
	for (EDirection Direction : NavigationGlobals::Directions4)
	{
		const FGridCellPosition NeighbourPosition = DirectionToCellPosition(Direction) + CellPosition;
		OutNeighbours.Add(NeighbourPosition);
	}
}

void UGridUtilsFunctionLibrary::ForEachGridCell(const FGridSizes& GridSizes, const TFunction<void(const FGridCellPosition&)>& Callback)
{
	for (int32 Row = 0; Row < GridSizes.Rows; Row++)
	{
		for (int32 Col = 0; Col < GridSizes.Cols; Col++)
		{
			Callback(FGridCellPosition{ Col, Row });
		}
	}
}

void UGridUtilsFunctionLibrary::ForEachGridCell(const FGridBounds& GridBounds, const TFunction<void(const FGridCellPosition&)>& Callback)
{
	for (int32 Row = GridBounds.BottomLeftCell.Y; Row < GridBounds.TopRightCell.Y; Row++)
	{
		for (int32 Col = GridBounds.BottomLeftCell.X; Col < GridBounds.TopRightCell.X; Col++)
		{
			Callback(FGridCellPosition{ Col, Row });
		}
	}
}

void UGridUtilsFunctionLibrary::ForEachGridCellOnPerimeter(const FGridCellPosition& CenterCell, int32 Offset,
	const TFunction<void(const FGridCellPosition&)>& Callback)
{
	for (int32 Row = -Offset; Row <= Offset; Row++)
	{
		for (int32 Col = -Offset; Col < Offset; Col++)
		{
			if (Row != -Offset && Row != Offset && Col != -Offset && Col != Offset)
			{
				continue;
			}
			
			Callback(CenterCell + FGridCellPosition{ Col, Row });
		}
	}
}
