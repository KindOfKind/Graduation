// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Flowfield.h"

#include "Flowfield/GoalPoint.h"
#include "Flowfield/Components/FlowfieldCalculatorComponent.h"
#include "Flowfield/Misc/GridsFunctionsLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"


AFlowfield::AFlowfield()
{
	GridSettings.CellSize = 100;
	GridSettings.GridSizes = {100, 100};
	bCostsGridPendingRecalculation = false;

	FlowfieldCalculatorComponent = CreateDefaultSubobject<UFlowfieldCalculatorComponent>("FlowfieldCalculatorComponent");
	//AddOwnedComponent(FlowfieldCalculatorComponent);
}

void AFlowfield::BeginPlay()
{
	Super::BeginPlay();
}

void AFlowfield::Initialize()
{
	RecalculateAllGrids();
}

void AFlowfield::RecalculateAllGrids()
{
	FlowfieldCalculatorComponent->RecalculateAllGrids();
}

void AFlowfield::RecalculateCostsGrid()
{
	FlowfieldCalculatorComponent->RecalculateCostsGrid();
}

void AFlowfield::CalculateDirectionsGridToGoalPoint(TSharedPtr<FDirectionsGrid> DirectionsGrid, AGoalPoint* GoalPoint)
{
	FlowfieldCalculatorComponent->CalculateDirectionsGridToGoalPoint(DirectionsGrid, GoalPoint);
}

void AFlowfield::RecalculateDirectionsGrids()
{
	FlowfieldCalculatorComponent->RecalculateDirectionsGrids();
}

void AFlowfield::RecalculateDirectionsGrids(TArray<AGoalPoint*>& InGoalPoints)
{
	FlowfieldCalculatorComponent->RecalculateDirectionsGrids(InGoalPoints);
}

FVector AFlowfield::GetCenter()
{
	float Width = GridSettings.CellSize * GridSettings.GridSizes.Cols;
	float Height = GridSettings.CellSize * GridSettings.GridSizes.Rows;
	return GetActorLocation() + FVector{Width / 2.f, Height / 2.f, 0.f};
}

void AFlowfield::GetGridBounds(FGridBounds& OutBounds) const
{
	UGridsFunctionsLibrary::GetGridAreaBounds(OutBounds, GetActorLocation(), GridSettings.GridSizes, GridSettings.CellSize);
}

FVector AFlowfield::GetDirectionAtLocation(const FVector& Location, int32 DirectionsGridIndex, bool bUseDetour)
{
	if (!DirectionsGrids.IsValidIndex(DirectionsGridIndex))
	{
		return FDirectionsGrid::NONE_DIRECTION;
	}

	return GetDirectionAtCell(UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, GridSettings.CellSize), DirectionsGridIndex, bUseDetour);
}

FVector AFlowfield::GetDirectionAtCell(const FGridCellPosition& CellPosition, int32 DirectionsGridIndex, bool bUseDetour)
{
	TSharedPtr<FDirectionsGrid> DirGrid = DirectionsGrids[DirectionsGridIndex];
	if (bUseDetour && DirGrid->DetourDirectionsGrid)
	{
		DirGrid = DirGrid->DetourDirectionsGrid;
	}

	return DirGrid->GetDirection(CellPosition);
}

AGoalPoint* AFlowfield::GetGoalPoint(int32 DirectionGridIndex)
{
	return GoalPoints[DirectionGridIndex];
}

bool AFlowfield::DoesContainCell(const FGridCellPosition& CellPosition) const
{
	FGridBounds FlowfieldBounds;
	GetGridBounds(FlowfieldBounds);
	return FlowfieldBounds.IsCellInBounds(CellPosition);
}

