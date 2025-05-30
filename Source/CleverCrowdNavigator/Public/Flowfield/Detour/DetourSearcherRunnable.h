// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Collisions/CCSCollisionsHashGrid.h"
#include "Flowfield/GridTypes.h"

// 1st param - detour directions grids
// 2nd param - dense crowd areas
DECLARE_DELEGATE_TwoParams(FCalculatedDetoursSignature, TArray<TSharedPtr<FDirectionsGrid>>, TArray<FGridBounds>)


class CLEVERCROWDNAVIGATOR_API FDetourSearcherRunnable : public FRunnable
{

public:
	struct GoalInfo
	{
		FGridCellPosition CellPosition;
		FGridSizes GridSizes;
	};
	// Data used for Detour Paths calculation
	struct FPayload
	{
		FCostsGrid CostsGrid;                  // To increase costs in areas with high collisions count and calculate directions grids
		FCCSCollisionsHashGrid CollisionsGrid; // To look up areas with high collisions count
		TArray<GoalInfo> GoalsInfos;            
	} Payload;

	FCalculatedDetoursSignature CalculatedDetoursDelegate;

	// CONFIG START
	int32 MaxAdditionalCost = 40;
	// CONFIG END
	
protected:
	std::atomic<bool> bWorkDone      = false;
	std::atomic<bool> bPayloadPassed = false;
	std::atomic<bool> bWorkStarted   = false;

	FCostsGrid ModifiedCostsGrid;
	TArray<TSharedPtr<FDirectionsGrid>> DetourDirectionsGrids;
	TArray<FGridBounds> DenseAreas;
	
private:
	FRunnableThread* Thread = nullptr;

public:
	FDetourSearcherRunnable(EThreadPriority ThreadPriority = EThreadPriority::TPri_Normal, const int32 StackSize = 0);
	virtual ~FDetourSearcherRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	virtual FRunnableThread* GetThread();

	void StartCalculation(const FPayload& InPayload);

protected:

	virtual void Main();
	void CommitDetourCalculationFinish();

private:

	void UpdateCostsInDenseArea(const FGridBounds& DenseArea);
	void UpdateCostInCell(const FGridCellPosition& CellPosition, const int32 OuterAdditiveCost = 0);
};
