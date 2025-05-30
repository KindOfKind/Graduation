// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapAnalyzer/MapClusterizationTypes.h"
#include "UObject/Object.h"
#include "MapClusterDefinerBase.generated.h"

struct FGridCellPosition;
class UMapAnalyzerSubsystem;

/**
 * Class that allows us to implement different methods of defining which "type" should be assigned to a map cluster 
 */
UCLASS()
class CLEVERCROWDSIM_API UMapClusterDefinerBase : public UObject
{
	GENERATED_BODY()

	friend UMapAnalyzerSubsystem;

public:
	typedef CCSMapClustering::FClusterType FClusterType;
	
protected:
	UPROPERTY()
	TObjectPtr<UMapAnalyzerSubsystem> MapAnalyzer;

public:
	// Defines cluster type by a set of points
	virtual FClusterType DefineClusterType(const int32 ClusterID, const TArray<FGridCellPosition>& NonEmptyCells);

private:
	void Initialize(UMapAnalyzerSubsystem* InMapAnalyzerSubsystem);
};
