// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Misc/FlowfieldCalculationFunctionsLibrary.h"

#include "Flowfield/GridTypes.h"
#include "Flowfield/Misc/GridsFunctionsLibrary.h"
#include "Flowfield/NavigationAffector/Interfaces/NavigationAffector.h"
#include "Grids/GridUtilsFunctionLibrary.h"

void UFlowfieldCalculationFunctionsLibrary::CalculateIntegrationGrid(FIntegrationGrid& OutIntegrationGrid, const FGridCellPosition& GoalCellPosition,
                                                                     const FGridBounds& GridBounds, const FCostsGrid& CostsGrid)
{
	const FGridCellPosition& GoalPos = GoalCellPosition;
	
	TQueue<FGridCellPosition> CellsQueue;
	CellsQueue.Enqueue(GoalPos);
	OutIntegrationGrid.SetCost(GoalPos, 1);

	FGridCellPosition CurrentCellPos;
	while (CellsQueue.Dequeue(CurrentCellPos))
	{
		const float CurrentTotalCost = OutIntegrationGrid.GetCost(CurrentCellPos);
		
		TArray<FGridCellPosition> NeighbourCells;
		UGridsFunctionsLibrary::GetGridCellValidNeighbours8(NeighbourCells, CostsGrid, CurrentCellPos);

		for (FGridCellPosition& NeighbourCellPos : NeighbourCells)
		{
		if (!GridBounds.IsCellInBounds(NeighbourCellPos) || !OutIntegrationGrid.Cells.Contains(NeighbourCellPos))
			{
				continue;
			}
			
			float CostMult = 1.f;
			// Diagonal path is longer by 1.41f
			const FGridCellPosition NeighbourDirection = CurrentCellPos - NeighbourCellPos;
			if (NeighbourDirection.X != 0 && NeighbourDirection.Y != 0)
			{
				CostMult *= 1.41f;
			}
			
			const float MoveToNeighbourCost = CostsGrid.GetCost(NeighbourCellPos) * CostMult;
			const float NewNeighbourTotalCost = CurrentTotalCost + MoveToNeighbourCost;

			const float NeighbourCost = OutIntegrationGrid.GetCost(NeighbourCellPos);	// -1 in Integration field means the cell is not processed yet
			if (NewNeighbourTotalCost < NeighbourCost || NeighbourCost == -1)			// If we found a shorter path to the neighbour cell
			{
				OutIntegrationGrid.SetCost(NeighbourCellPos, NewNeighbourTotalCost);
				CellsQueue.Enqueue(NeighbourCellPos);
			}
		}
	}
}

void UFlowfieldCalculationFunctionsLibrary::CalculateDirectionsGrid(FDirectionsGrid& OutDirectionsGrid, const FCostsGrid& CostsGrid,
                                                                    const FGridCellPosition& GoalCellPosition, const FGridSizes& GridSizes)
{
	if (OutDirectionsGrid.bCalculated)
	{
		UE_LOG(LogTemp, Error, TEXT("Direction Grid has already been calculated."));
		return;
	}
	
	FGridCellPosition BottomLeftCell = {GoalCellPosition.X - (GridSizes.Cols / 2), GoalCellPosition.Y - (GridSizes.Rows / 2)};
	FGridCellPosition TopRightCell   = {GoalCellPosition.X + (GridSizes.Cols / 2), GoalCellPosition.Y + (GridSizes.Rows / 2)};
	FGridBounds GridBounds{BottomLeftCell, TopRightCell};

	// Each Integration Grid cell contains a distance from this cell to the goal cell.
	FIntegrationGrid IntegrationGrid;
	UGridUtilsFunctionLibrary::ForEachGridCell(GridBounds, [&IntegrationGrid, &CostsGrid](const FGridCellPosition& CellPosition)
	{
		if (CostsGrid.Bounds.IsCellInBounds(CellPosition))
		{
			IntegrationGrid.AddCell(CellPosition, -1);	// -1 in Integration field means the cell is not processed yet
		}
	});
	
	CalculateIntegrationGrid(IntegrationGrid, GoalCellPosition, GridBounds, CostsGrid);

	OutDirectionsGrid.Cells.Reserve(IntegrationGrid.Cells.Num());

	// Iterate through each cell of the integration grid and set its direction towards a neighbour with the lowest value
	for (auto& [CellPosition, Cell] : IntegrationGrid.Cells)
	{
		float const* ClosestCellValue;
		EDirection DirectionToClosestCell = EDirection::Top;
		UGridsFunctionsLibrary::GetGridCellClosestNeighbour8(ClosestCellValue, DirectionToClosestCell, IntegrationGrid, CellPosition);
		if (!ClosestCellValue)
		{
			OutDirectionsGrid.Cells.Add(CellPosition, FDirectionsGridCell{});
			continue;
		}

		const FVector Direction = UGridsFunctionsLibrary::DirectionToVector(DirectionToClosestCell);
		OutDirectionsGrid.Cells.Add(CellPosition, FDirectionsGridCell{Direction});
	}

	OutDirectionsGrid.bCalculated = true;
}

int32 UFlowfieldCalculationFunctionsLibrary::GetNavigationAffectorCost(AActor* Actor)
{
	check(Actor);
	if (!Actor->Implements<UNavigationAffector>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Navigation Affector actor has to implement Navigation Effector interface."));
		return UE::NavigationGlobals::MaxCost;
	}

	return INavigationAffector::Execute_GetFlowfieldNavigationCost(Actor);
}
