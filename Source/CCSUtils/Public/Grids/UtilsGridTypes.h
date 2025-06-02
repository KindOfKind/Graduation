// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UtilsGridTypes.generated.h"


UENUM(BlueprintType)
enum class EDirection : uint8
{
	Top = 0,
	TopRight,
	Right,
	BottomRight,
	Bottom,
	BottomLeft,
	Left,
	TopLeft,

	Max
};
ENUM_RANGE_BY_COUNT(EDirection, EDirection::Max)


namespace NavigationGlobals
{
	// Matches EDirection enum values
	inline const TArray<FVector> DirectionVectors =
	{
		{0.f, 1.f, 0.f},
		{1.f, 1.f, 0.f},
		{1.f, 0.f, 0.f},
		{1.f, -1.f, 0.f},
		{0.f, -1.f, 0.f},
		{-1.f, -1.f, 0.f},
		{-1.f, 0.f, 0.f},
		{-1.f, 1.f, 0.f}
	};

	inline const TArray<EDirection> Directions4 =
	{
		EDirection::Top, EDirection::Right, EDirection::Bottom, EDirection::Left
	};
}


USTRUCT(BlueprintType)
struct CCSUTILS_API FGridCellPosition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 X = 0;
	UPROPERTY(EditAnywhere)
	int32 Y = 0;
	
	FGridCellPosition() = default;
	FGridCellPosition(const int32& InX, const int32& InY) : X(InX), Y(InY) {};

	FGridCellPosition operator+(const FGridCellPosition& Other) const
	{
		return FGridCellPosition(X + Other.X, Y + Other.Y);
	}
	FGridCellPosition operator-(const FGridCellPosition& Other) const
	{
		return FGridCellPosition(X - Other.X, Y - Other.Y);
	}

	bool operator== (const FGridCellPosition& Other) const
	{
		return (X == Other.X) && (Y == Other.Y);
	}

	friend inline uint32 GetTypeHash(const FGridCellPosition& CellPosition)
	{
		return HashCombine(GetTypeHash(CellPosition.X), GetTypeHash(CellPosition.Y));
	}

	friend FArchive& operator <<(FArchive& Ar, FGridCellPosition& CellPosition)
	{
		Ar << CellPosition.X;
		Ar << CellPosition.Y;
		return Ar;
	}
};


USTRUCT(BlueprintType)
struct CCSUTILS_API FGridSizes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rows = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cols = 0;
	
	FGridSizes() = default;
	FGridSizes(const int32& InSize) : Rows(InSize), Cols(InSize) {};
	FGridSizes(const int32& InRows, const int32& InCols) : Rows(InRows), Cols(InCols) {};

	bool operator== (const FGridSizes& Other) const
	{
		return (Rows == Other.Rows) && (Cols == Other.Cols);
	}

	friend inline uint32 GetTypeHash(const FGridSizes& GridSizes)
	{
		return HashCombine(GetTypeHash(GridSizes.Rows), GetTypeHash(GridSizes.Cols));
	}
};


// Describes box bound in grid (considering the grid is filled with cells in order: from left to right, from bottom to top)
USTRUCT()
struct CCSUTILS_API FGridBounds
{
	GENERATED_BODY()

	// X and Y (col and row) indices of the bottom left cell
	UPROPERTY(EditAnywhere)
	FGridCellPosition BottomLeftCell;
	// X and Y (col and row) indices of the up right cell
	UPROPERTY(EditAnywhere)
	FGridCellPosition TopRightCell;
	
	FGridBounds() = default;
	FGridBounds(const FGridCellPosition& InBottomLeftCell, const FGridCellPosition& InUpRightCell) : BottomLeftCell(InBottomLeftCell), TopRightCell(InUpRightCell) {};

	bool operator== (const FGridBounds& Other) const
	{
		return (BottomLeftCell == Other.BottomLeftCell) && (TopRightCell == Other.TopRightCell);
	}

	friend inline uint32 GetTypeHash(const FGridBounds& GridBounds)
	{
		return HashCombine(GetTypeHash(GridBounds.BottomLeftCell), GetTypeHash(GridBounds.TopRightCell));
	}

	bool IsCellInBounds(const FGridCellPosition& CellPosition) const
	{
		return (CellPosition.X >= BottomLeftCell.X) && (CellPosition.Y >= BottomLeftCell.Y) &&
			(CellPosition.X <= TopRightCell.X) && (CellPosition.Y <= TopRightCell.Y);
	}

	void UpdateToFitCell(const FGridCellPosition& CellPosition)
	{
		if (CellPosition.X < BottomLeftCell.X)
		{
			BottomLeftCell.X = CellPosition.X;
		}
		else if (CellPosition.X > TopRightCell.X)
		{
			TopRightCell.X = CellPosition.X;
		}
		if (CellPosition.Y < BottomLeftCell.Y)
		{
			BottomLeftCell.Y = CellPosition.Y;
		}
		else if (CellPosition.Y > TopRightCell.Y)
		{
			TopRightCell.Y = CellPosition.Y;
		}
	}

	void MergeWithBounds(const FGridBounds& OtherBounds)
	{
		UpdateToFitCell(OtherBounds.BottomLeftCell);
		UpdateToFitCell(OtherBounds.TopRightCell);
	}

	uint64 GetArea() const
	{
		return abs((TopRightCell.X - BottomLeftCell.X + 1) * (TopRightCell.Y - BottomLeftCell.Y + 1));
	}

	int32 GetIntersectionArea(const FGridBounds& Other) const
	{
		int32 InterLeft = std::max(BottomLeftCell.X, Other.BottomLeftCell.X);
		int32 InterBottom = std::max(BottomLeftCell.Y, Other.BottomLeftCell.Y);
		int32 InterRight = std::min(TopRightCell.X, Other.TopRightCell.X);
		int32 InterTop = std::min(TopRightCell.Y, Other.TopRightCell.Y);

		int32 InterWidth = InterRight - InterLeft;
		int32 InterHeight = InterTop - InterBottom;

		if (InterWidth <= 0 || InterHeight <= 0) {
			return 0;
		}

		return InterWidth * InterHeight;
	}

	FVector GetExtent(const float CellSize) const
	{
		return FVector{(TopRightCell.X - BottomLeftCell.X + 1) * CellSize / 2.f, (TopRightCell.Y - BottomLeftCell.Y + 1) * CellSize / 2.f, 1.f};
	}

	FGridCellPosition GetCenterCell() const
	{
		return FGridCellPosition{(TopRightCell.X + BottomLeftCell.X) / 2, (TopRightCell.Y + BottomLeftCell.Y) / 2};
	}

	void Expand(int32 CellsNum)
	{
		BottomLeftCell = BottomLeftCell - FGridCellPosition{2, 2};
		TopRightCell   = TopRightCell + FGridCellPosition{2, 2};
	}
};