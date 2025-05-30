// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapClusterDefinerBase.h"
#include "SimpleMapClusterDefiner.generated.h"

/**
 * 
 */
UCLASS()
class CLEVERCROWDSIM_API USimpleMapClusterDefiner : public UMapClusterDefinerBase
{
	GENERATED_BODY()

public:

	virtual FClusterType DefineClusterType(const int32 ClusterID, const TArray<FGridCellPosition>& NonEmptyCells) override;
};
