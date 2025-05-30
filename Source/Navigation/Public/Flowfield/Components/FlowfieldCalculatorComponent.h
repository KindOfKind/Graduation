// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlowfieldCalculatorComponent.generated.h"


struct FDirectionsGrid;
class AGoalPoint;
class AFlowfield;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAVIGATION_API UFlowfieldCalculatorComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	// Flowfield that owns this component
	AFlowfield* Flowfield;

public:
	UFlowfieldCalculatorComponent();

protected:
	virtual void BeginPlay() override;

public:

	void RecalculateAllGrids();
	// Initialized costs in all cells of the grid by "scanning" the whole map.
	void RecalculateCostsGrid();

	void CalculateDirectionsGridToGoalPoint(TSharedPtr<FDirectionsGrid> DirectionsGrid, AGoalPoint* GoalPoint);
	// Finds all Goal Points on the map and calculates Directions Grids to them.
	void RecalculateDirectionsGrids();
	// Destroys old Direction Grids and calculates new ones to the Goal Points.
	void RecalculateDirectionsGrids(TArray<AGoalPoint*>& GoalPoints);
};
