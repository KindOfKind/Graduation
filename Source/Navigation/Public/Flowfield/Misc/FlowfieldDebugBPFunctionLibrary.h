// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlowfieldDebugBPFunctionLibrary.generated.h"


struct FGridCellPosition;
class AFlowfield;

UCLASS()
class NAVIGATION_API UFlowfieldDebugBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Flowfield|Debug")
	static void DrawDebugCostsGridInRadius(AFlowfield* Flowfield, FVector DebugCenter, float Radius = 500.f, float DebugLifetime = 4.f);

	UFUNCTION(BlueprintCallable, Category = "Flowfield|Debug")
	static void DrawDebugDirectionsGridInRadius(AFlowfield* Flowfield, int32 DirectionGridIndex, bool bDetour, FVector DebugCenter, float Radius = 500.f, float DebugLifetime = 4.f);

	static void DrawDebugLineAtFlowfieldCell(AFlowfield* Flowfield, const FGridCellPosition& CellPosition, float LineLength = 1000.f, FColor Color = FColor::Red, float DebugLifetime = 4.f);
};
