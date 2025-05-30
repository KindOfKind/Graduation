// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Detour/DetourSearcherRunnable.h"

#include "Flowfield/Misc/FlowfieldCalculationFunctionsLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"


FDetourSearcherRunnable::FDetourSearcherRunnable(EThreadPriority ThreadPriority, const int32 StackSize)
{
	Thread = FRunnableThread::Create(this, TEXT("Flowfield Detour Searcher"), StackSize, ThreadPriority);
}

FDetourSearcherRunnable::~FDetourSearcherRunnable()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

bool FDetourSearcherRunnable::Init()
{
	return true;
}

uint32 FDetourSearcherRunnable::Run()
{
	while (!bWorkDone.load())
	{
		if (bPayloadPassed.load() && !bWorkStarted.load())
		{
			Main();
		}
	}

	return 0;
}

void FDetourSearcherRunnable::Exit()
{
}

void FDetourSearcherRunnable::Stop()
{
	bWorkDone.store(true);
}

FRunnableThread* FDetourSearcherRunnable::GetThread()
{
	return Thread;
}

void FDetourSearcherRunnable::StartCalculation(const FPayload& InPayload)
{
	Payload = InPayload;
	ModifiedCostsGrid = Payload.CostsGrid;
	bPayloadPassed.store(true);
}

void FDetourSearcherRunnable::Main()
{
	bWorkStarted.store(true);

	// Search for dense crowd areas
	Payload.CollisionsGrid.SearchForDenseAreas(DenseAreas);
	
	// Update ModifiedCostsGrid costs in dense crowd areas
	for (const FGridBounds& DenseArea : DenseAreas)
	{
		UpdateCostsInDenseArea(DenseArea);
	}

	// Calculate detour directions grids using new costs
	for (int32 GoalIdx = 0; GoalIdx < Payload.GoalsInfos.Num(); GoalIdx++)
	{
		TSharedPtr<FDirectionsGrid> NewDirectionsGrid = MakeShareable<FDirectionsGrid>(new FDirectionsGrid());
		UFlowfieldCalculationFunctionsLibrary::CalculateDirectionsGrid(*NewDirectionsGrid.Get(), ModifiedCostsGrid,
		                                                               Payload.GoalsInfos[GoalIdx].CellPosition, Payload.GoalsInfos[GoalIdx].GridSizes);
		DetourDirectionsGrids.Add(NewDirectionsGrid);
	}

	// Send detour directions grids through game thread
	FSimpleDelegateGraphTask::CreateAndDispatchWhenReady
	(
		FSimpleDelegateGraphTask::FDelegate::CreateRaw(this, &FDetourSearcherRunnable::CommitDetourCalculationFinish),
		TStatId(), nullptr, ENamedThreads::GameThread
	);
}

void FDetourSearcherRunnable::CommitDetourCalculationFinish()
{
	bool bCalledBack = CalculatedDetoursDelegate.ExecuteIfBound(DetourDirectionsGrids, DenseAreas);
	
	bWorkDone.store(true);
}

void FDetourSearcherRunnable::UpdateCostsInDenseArea(const FGridBounds& DenseArea)
{
	UGridUtilsFunctionLibrary::ForEachGridCell(DenseArea, [this](const FGridCellPosition& CellPosition)
	{
		UpdateCostInCell(CellPosition);
	});
}

void FDetourSearcherRunnable::UpdateCostInCell(const FGridCellPosition& CellPosition, const int32 OuterAdditiveCost)
{
	const FVector2f CollisionsRange   = FVector2f{0, static_cast<float>(Payload.CollisionsGrid.MaxCollisionsCount)};
	const FVector2f AdditiveCostRange = FVector2f{0, static_cast<float>(MaxAdditionalCost)};
	const int32 CollisionsCount       = Payload.CollisionsGrid.GetCollisionsCountAtCell(CellPosition);
	const int32 AdditionalCost        = FMath::GetMappedRangeValueClamped(CollisionsRange, AdditiveCostRange, CollisionsCount);
	const int32 ModifiedCost          = Payload.CostsGrid.GetCostSafe(CellPosition) + AdditionalCost + OuterAdditiveCost;

	if (ModifiedCostsGrid.GetCostSafe(CellPosition) < ModifiedCost)
	{
		ModifiedCostsGrid.SetCost(CellPosition, ModifiedCost);
	}
}
