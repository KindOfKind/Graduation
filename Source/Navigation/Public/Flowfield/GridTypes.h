// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global/NavigationGlobals.h"
#include "Grids/UtilsGridTypes.h"
#include "GridTypes.generated.h"

class AGoalPoint;


USTRUCT(BlueprintType)
struct NAVIGATION_API FFlowfieldGridSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridSizes GridSizes;
};


USTRUCT()
struct NAVIGATION_API FIntegrationGrid
{
	GENERATED_BODY()

	TMap<FGridCellPosition, float> Cells;
	
	FIntegrationGrid() = default;
	FIntegrationGrid(const FIntegrationGrid& InGrid) : Cells(InGrid.Cells) {};

	void AddCell(const FGridCellPosition& Position, const float& Value)
	{
		Cells.Add(Position, Value);
	};

	void SetCost(const FGridCellPosition& Position, const float& Value)
	{
		Cells[Position] = Value;
	}
	
	float GetCost(const FGridCellPosition& Position) const
	{
		return Cells[Position];
	}
};


USTRUCT()
struct NAVIGATION_API FCostsGridCell
{
	GENERATED_BODY()
	
	uint8 Cost;
};

USTRUCT()
struct NAVIGATION_API FCostsGrid
{
	GENERATED_BODY()

	// If some cell is not in the map, then it's outside the Flowfield.
	TMap<FGridCellPosition, FCostsGridCell> Cells;
	FGridBounds Bounds;
	
	FCostsGrid() = default;
	FCostsGrid(const FCostsGrid& InGrid) : Cells(InGrid.Cells) {};

	void AddCell(const FGridCellPosition& Position, const uint8& Cost)
	{
		Cells.Add(Position, FCostsGridCell{Cost});
		Bounds.UpdateToFitCell(Position);
	};

	// Sets cost in all existing cells to max value
	void ResetCosts()
	{
		for (auto& [CellPosition, Cell] : Cells)
		{
			Cell.Cost = UE::NavigationGlobals::MaxCost;
		}
	}

	void SetCostChecked(const FGridCellPosition& Position, const uint8& Value)
	{
		Cells[Position] = FCostsGridCell{Value};
	}
	// If no cell at Position, will do nothing.
	void SetCost(const FGridCellPosition& Position, const uint8& Value)
	{
		if (FCostsGridCell* CostCell = Cells.Find(Position))
		{
			CostCell->Cost = Value;
			Bounds.UpdateToFitCell(Position);
		}
	}
	
	float GetCost(const FGridCellPosition& Position) const
	{
		return Cells[Position].Cost;
	}
	float GetCostSafe(const FGridCellPosition& Position) const
	{
		if (const FCostsGridCell* CostCell = Cells.Find(Position))
		{
			return CostCell->Cost;
		}
		return UE::NavigationGlobals::MaxCost;
	}

	bool Contains(const FGridCellPosition& Position) const
	{
		return Cells.Contains(Position);
	}
};


USTRUCT()
struct NAVIGATION_API FDirectionsGridCell
{
	GENERATED_BODY()
	
	FVector Direction = {0.f, 0.f, 0.f};
};

USTRUCT()
struct NAVIGATION_API FDirectionsGrid
{
	GENERATED_BODY()

	inline static const FVector NONE_DIRECTION = FVector::ZeroVector;

	UPROPERTY()
	TMap<FGridCellPosition, FDirectionsGridCell> Cells;

	UPROPERTY()
	TObjectPtr<AGoalPoint> GoalPoint;

	bool bCalculated = false;

	TSharedPtr<FDirectionsGrid> DetourDirectionsGrid;
	
	FDirectionsGrid() = default;
	FDirectionsGrid(const FDirectionsGrid& InGrid) : Cells(InGrid.Cells) {};

	FVector GetDirectionChecked(const FGridCellPosition& Position) const
	{
		return Cells[Position].Direction;
	}
	FVector GetDirection(const FGridCellPosition& Position) const
	{
		if (const FDirectionsGridCell* DirectionCell = Cells.Find(Position))
		{
			return DirectionCell->Direction;
		}
		return NONE_DIRECTION;
	}
};
