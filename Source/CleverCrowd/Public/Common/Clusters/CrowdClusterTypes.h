// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "CrowdClusterTypes.generated.h"


namespace CCSCrowdCluster
{
	typedef int32 FClusterType;

	// Should be identical to ClusterTypes from CleverCrowdSim module
	namespace ClusterTypes
	{
		constexpr FClusterType Undefined = 0;
		constexpr FClusterType Spacious = 1;
		constexpr FClusterType Dense = 2;
	}
}


USTRUCT()
struct CLEVERCROWD_API FClusterFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ClusterType;	// In what type of map cluster (e.g. spacious or dense area) the entity currently is
	UPROPERTY()
	int32 AreaId;
};