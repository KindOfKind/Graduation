// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Components/FlowfieldCalculatorComponent.h"

#include "Flowfield/Flowfield.h"
#include "Flowfield/GoalPoint.h"
#include "Flowfield/Misc/FlowfieldCalculationFunctionsLibrary.h"
#include "Flowfield/Misc/GridsFunctionsLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


UFlowfieldCalculatorComponent::UFlowfieldCalculatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Flowfield = nullptr;
}


void UFlowfieldCalculatorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("%hs Failed to get owner"), __FUNCTION__);
		return;
	}
	
	Flowfield = Cast<AFlowfield>(GetOwner());
}

void UFlowfieldCalculatorComponent::RecalculateAllGrids()
{
	RecalculateCostsGrid();
	RecalculateDirectionsGrids();
}

void UFlowfieldCalculatorComponent::RecalculateCostsGrid()
{
	Flowfield->CostsGrid = MakeShared<FCostsGrid>();
	
	constexpr float TraceStartHeight = 10000.f;
	constexpr float TraceEndHeight   = -10000.f;

	UE_LOG(LogTemp, Display, TEXT("TEST. Flowfield Center Cell: %s"), *Flowfield->GetActorLocation().ToString());

	FGridBounds FlowfieldGridBounds;
	const int32 GridCellSize = Flowfield->GridSettings.CellSize;
	UGridsFunctionsLibrary::GetGridAreaBounds(FlowfieldGridBounds, Flowfield->GetActorLocation(), Flowfield->GridSettings.GridSizes, GridCellSize);
	
	// Sweep cubes of CellSize in each cell location to check if a walkable surface is there.
	// @note You can add "Walkable Surface type" checks to assign different costs in different areas of the map. 
	UGridUtilsFunctionLibrary::ForEachGridCell(FlowfieldGridBounds, [this, GridCellSize](const FGridCellPosition& CellPosition)
	{
		FHitResult HitResults;
		const FVector CellLocation = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, GridCellSize);
		const FCollisionShape TraceShape = FCollisionShape::MakeBox(FVector(GridCellSize, GridCellSize, GridCellSize));

		GetWorld()->SweepSingleByChannel(HitResults,
										CellLocation + FVector(0, 0, TraceStartHeight),
										CellLocation + FVector(0, 0, TraceEndHeight),
										FQuat{}, UE::NavigationGlobals::NavigationAffectorChannel, TraceShape);

		uint8 CellCost = UE::NavigationGlobals::MaxCost;
		if (HitResults.IsValidBlockingHit() && IsValid(HitResults.GetActor()))
		{
			CellCost = UFlowfieldCalculationFunctionsLibrary::GetNavigationAffectorCost(HitResults.GetActor());
		}

		Flowfield->CostsGrid.Get()->AddCell(CellPosition, CellCost);
	});
}

void UFlowfieldCalculatorComponent::CalculateDirectionsGridToGoalPoint(TSharedPtr<FDirectionsGrid> DirectionsGrid, AGoalPoint* GoalPoint)
{
	check (IsValid(GoalPoint));
	check (Flowfield->CostsGrid.IsValid());

	FGridCellPosition GoalCellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(GoalPoint->GetActorLocation(), Flowfield->GridSettings.CellSize);
	
	FDirectionsGrid NewDirectionsGrid;
	DirectionsGrid->GoalPoint = GoalPoint;
	UFlowfieldCalculationFunctionsLibrary::CalculateDirectionsGrid(*DirectionsGrid.Get(), *Flowfield->CostsGrid.Get(), GoalCellPosition, GoalPoint->GridSizes);

	GoalPoint->OwningDirectionsGrid = DirectionsGrid;
}

void UFlowfieldCalculatorComponent::RecalculateDirectionsGrids()
{
	Flowfield->DirectionsGrids.Empty();
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoalPoint::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AGoalPoint* GoalPoint = Cast<AGoalPoint>(Actor);
		check(GoalPoint);

		TSharedPtr<FDirectionsGrid> NewDirectionsGrid = MakeShareable<FDirectionsGrid>(new FDirectionsGrid());
		CalculateDirectionsGridToGoalPoint(NewDirectionsGrid, GoalPoint);
		Flowfield->DirectionsGrids.Add(NewDirectionsGrid);
		Flowfield->GoalPoints.Add(GoalPoint);
	}
}

void UFlowfieldCalculatorComponent::RecalculateDirectionsGrids(TArray<AGoalPoint*>& GoalPoints)
{
	Flowfield->DirectionsGrids.Empty();
	
	for (AGoalPoint* GoalPoint : GoalPoints)
	{
		check(GoalPoint);

		TSharedPtr<FDirectionsGrid> NewDirectionsGrid = MakeShareable<FDirectionsGrid>(new FDirectionsGrid());
		CalculateDirectionsGridToGoalPoint(NewDirectionsGrid, GoalPoint);
		Flowfield->DirectionsGrids.Add(NewDirectionsGrid);
	}
}

