// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MapAreaAnalyzer.generated.h"


USTRUCT()
struct CLEVERCROWDSIM_API FMapAreaData
{
	GENERATED_BODY()

	inline static int32 FloatParamsNum = 3;	// ToDo: change to 4 when CornersCountInIsland calculation is implemented

	// Ratio of empty cells to all cells.
	UPROPERTY()
	float ObstacleCellsFraction = 0.f;
	
	// Average cells number in one obstacles island.
	UPROPERTY()
	float ObstaclesIslandSize = 0.f;

	// Average of how close different islands to each other (along the whole island length)
	UPROPERTY()
	float IslandsProximity = 0.f;

	// Average number of corners in one island.
	UPROPERTY()
	float CornersCountInIsland = 0.f;
	
	float GetParameterValue(const int32 ParameterIndex)
	{
		switch (ParameterIndex)
		{
		case 0:
			return ObstacleCellsFraction;
		case 1:
			return FMath::Clamp(ObstaclesIslandSize * 10.f, 0.f, 1.f);
		case 2:
			return IslandsProximity;
		case 3:
			return CornersCountInIsland;
		default:
			return ObstacleCellsFraction;
		}
	}

	friend FArchive& operator <<(FArchive& Ar, FMapAreaData& AreaData)
	{
		Ar << AreaData.ObstacleCellsFraction;
		Ar << AreaData.ObstaclesIslandSize;
		Ar << AreaData.IslandsProximity;
		Ar << AreaData.CornersCountInIsland;
		return Ar;
	}
};

/**
 * 
 */
UCLASS()
class CLEVERCROWDSIM_API UMapAreaAnalyzer : public UObject
{
	GENERATED_BODY()

public:
	static int32 GetAreaTypeId();
};
