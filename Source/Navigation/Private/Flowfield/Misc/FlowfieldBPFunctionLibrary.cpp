// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Misc/FlowfieldBPFunctionLibrary.h"

#include "Flowfield/Misc/GridsFunctionsLibrary.h"


FVector2D UFlowfieldBPFunctionLibrary::CalculateGridSize(const FGridSizes& GridSizes, const float& CellSize)
{
	return UGridsFunctionsLibrary::CalculateGridSize(GridSizes, CellSize);
}
