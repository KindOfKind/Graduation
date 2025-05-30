// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/ClusterDefiners/MapClusterDefinerBase.h"

UMapClusterDefinerBase::FClusterType UMapClusterDefinerBase::DefineClusterType(const int32 ClusterID, const TArray<FGridCellPosition>& NonEmptyCells)
{
	constexpr FClusterType DefaultClusterType = 0;
	return DefaultClusterType;
}

void UMapClusterDefinerBase::Initialize(UMapAnalyzerSubsystem* InMapAnalyzerSubsystem)
{
	MapAnalyzer = InMapAnalyzerSubsystem;
}
