// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Misc/FlowfieldDebugBPFunctionLibrary.h"

#include "Flowfield/Flowfield.h"
#include "Flowfield/Misc/GridsFunctionsLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UFlowfieldDebugBPFunctionLibrary::DrawDebugCostsGridInRadius(AFlowfield* Flowfield, FVector DebugCenter, float Radius, float DebugLifetime)
{
	if (!IsValid(Flowfield) || !Flowfield->CostsGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] Invalid data"), __FUNCTION__);
		if (IsValid(Flowfield))
		{
			UKismetSystemLibrary::PrintString(Flowfield->GetWorld(), "Costs grid is invalid", true, false, FColor::Red, 2.f);
		}
		return;
	}

	constexpr float DebugLineMaxLength = 1000.f;

	const int32 CellSize = Flowfield->GridSettings.CellSize;
	FGridBounds DebugAreaBounds;
	UGridsFunctionsLibrary::GetGridAreaBounds(DebugAreaBounds, DebugCenter, Radius, CellSize);

	UGridUtilsFunctionLibrary::ForEachGridCell(DebugAreaBounds, [&Flowfield, &CellSize, &DebugLifetime, &DebugCenter, DebugAreaBounds]
		(const FGridCellPosition& CellPosition)
	{
		FVector CellCenter = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, CellSize);
		
		if (!Flowfield->CostsGrid->Cells.Contains(CellPosition))
		{
			DrawDebugLine(Flowfield->GetWorld(), CellCenter, CellCenter + FVector::UpVector * DebugLineMaxLength, FColor::Green, false, DebugLifetime, 0, 3.f);
			return;
		}

		float Cost                = Flowfield->CostsGrid->Cells[CellPosition].Cost;
		float DebugLineLengthMult = FMath::GetMappedRangeValueClamped(
			FVector2D{0.f, static_cast<float>(UE::NavigationGlobals::MaxCost)}, FVector2D{0.2f, 1.f}, Cost);
		
		CellCenter.Z = DebugCenter.Z + 50.f;
		DrawDebugLine(Flowfield->GetWorld(), CellCenter, CellCenter + FVector::UpVector * DebugLineMaxLength * DebugLineLengthMult, FColor::Red, false, DebugLifetime, 0, 3.f);
	});
}

void UFlowfieldDebugBPFunctionLibrary::DrawDebugDirectionsGridInRadius(AFlowfield* Flowfield, int32 DirectionGridIndex, bool bDetour, FVector DebugCenter, float Radius, float DebugLifetime)
{
	if (!IsValid(Flowfield) || !Flowfield->DirectionsGrids.IsValidIndex(DirectionGridIndex) ||
		!Flowfield->DirectionsGrids[DirectionGridIndex].Get())
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] Invalid data"), __FUNCTION__);
		if (IsValid(Flowfield))
		{
			UKismetSystemLibrary::PrintString(Flowfield->GetWorld(), "Failed to draw direction grid debug", true, false, FColor::Red, 2.f);
		}
		return;
	}
	if (bDetour && !Flowfield->DirectionsGrids[DirectionGridIndex].Get()->DetourDirectionsGrid.Get())
	{
		UE_LOG(LogTemp, Error, TEXT("[%hs] Invalid Detour directions grid"), __FUNCTION__);
		return;
	}
	
	FDirectionsGrid& DirectionsGrid = (bDetour
		? *Flowfield->DirectionsGrids[DirectionGridIndex].Get()->DetourDirectionsGrid.Get()
		: *Flowfield->DirectionsGrids[DirectionGridIndex].Get());
	const int32 CellSize            = Flowfield->GridSettings.CellSize;
	
	FGridBounds DebugAreaBounds;
	UGridsFunctionsLibrary::GetGridAreaBounds(DebugAreaBounds, DebugCenter, Radius, CellSize);
	
	UGridUtilsFunctionLibrary::ForEachGridCell(DebugAreaBounds, [&Flowfield, &DirectionsGrid, &CellSize, &DebugLifetime, &DebugCenter, DebugAreaBounds]
		(const FGridCellPosition& CellPosition)
	{
		FVector CellCenter = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, CellSize);
		
		if (!DirectionsGrid.Cells.Contains(CellPosition))
		{
			DrawDebugLine(Flowfield->GetWorld(), CellCenter, CellCenter + FVector::UpVector * 500.f, FColor::Green, false, DebugLifetime, 0, 3.f);
			return;
		}

		const FVector CellDirection = DirectionsGrid.Cells[CellPosition].Direction;
		CellCenter.Z = DebugCenter.Z + 50.f;
		DrawDebugLine(Flowfield->GetWorld(), CellCenter, CellCenter + CellDirection * CellSize / 1.4f, FColor::Red, false, DebugLifetime, 0, 3.f);
		DrawDebugSphere(Flowfield->GetWorld(), CellCenter, 5.f, 6, FColor::Blue, false, DebugLifetime, 0, 2.f);
	});
	
}

void UFlowfieldDebugBPFunctionLibrary::DrawDebugLineAtFlowfieldCell(AFlowfield* Flowfield, const FGridCellPosition& CellPosition, float LineLength, FColor Color, float DebugLifetime)
{
	if (!IsValid(Flowfield)) return;
	
	FVector CellLocation = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPosition, Flowfield->GridSettings.CellSize);
	DrawDebugLine(Flowfield->GetWorld(), CellLocation, CellLocation + FVector::UpVector * LineLength, Color, false, DebugLifetime, 0, 6.f);
}
