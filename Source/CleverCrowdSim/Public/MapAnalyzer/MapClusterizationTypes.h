// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapClusterizationTypes.generated.h"


class UMapClusterDefinerBase;

namespace CCSMapClustering
{
	typedef int32 FClusterType;

	namespace ClusterTypes
	{
		constexpr FClusterType Undefined = 0;
		constexpr FClusterType Spacious = 1;
		constexpr FClusterType Dense = 2;
	}
}

USTRUCT(BlueprintType)
struct CLEVERCROWDSIM_API FClusterizationRules
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ClustersNum = 4;
	UPROPERTY()
	UMapClusterDefinerBase* ClusterDefiner;
	UPROPERTY()
	bool bDrawInitialPointsDebug = false;
	UPROPERTY()
	bool bDrawClusterIDsDebug = false;
	UPROPERTY()
	bool bDrawClusterTypesDebug = true;
};


USTRUCT()
struct CLEVERCROWDSIM_API FClusterCellData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ClusterID = -1;	// Used to distinguish different clusters
	UPROPERTY()
	int32 ClusterType = CCSMapClustering::ClusterTypes::Undefined;	// Used to define a certain type of cluster (e.g. dense or spacious area)
};
