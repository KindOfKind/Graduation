// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdStatisticsSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FUpdatedEntitiesCountSignature, int32 /*NewCount*/, int32 /*OldCount*/);

USTRUCT()
struct CLEVERCROWD_API FCrowdStatistics
{
	GENERATED_BODY()

	static constexpr int32 MaxClusterType = 3;	// @note: not less than max SectorType id.

	UPROPERTY()
	TArray<int32> AgentsInClusters;	// Index - SectorType id. Current num of entities in clusters.
	UPROPERTY()
	TArray<int32> AgentsInAreas;	// Index - Map Area Type id.
	UPROPERTY()
	TArray<int64> CollisionsInClusters;	// Index - SectorType id. Total num of collisions occured in clusters during a test.
	UPROPERTY()
	TArray<int64> CollisionsInAreas;
	UPROPERTY()
	TArray<float> ReachedFinishTimestamps;	// Timestamp is added when an agent reaches finish and leaves the level.
	UPROPERTY()
	TArray<float> AverageEntitySpeedInClusters;	// Index - SectorType id. Current average movement speed among all entities of each cluster.
	UPROPERTY()
	TArray<float> AverageEntitySpeedInAreas;

	FUpdatedEntitiesCountSignature UpdatedEntitiesCountDelegate;

private:
	int32 AgentsCounter = 0;

public:

	FCrowdStatistics()
	{
		AgentsInClusters.AddDefaulted(MaxClusterType);
		AgentsInAreas.AddDefaulted(10);	// temp
		CollisionsInClusters.AddDefaulted(MaxClusterType);
		CollisionsInAreas.AddDefaulted(10);	// temp
		AverageEntitySpeedInClusters.AddDefaulted(MaxClusterType);
		AverageEntitySpeedInAreas.AddDefaulted(10);	// temp
	}

	void UpdateAgentsCountersInClusters(const TArray<int32>& InAgentsInClusters)
	{
		const int32 OldAgentsCounter = AgentsCounter;
		AgentsInClusters             = InAgentsInClusters;
		AgentsCounter                = 0;
		for (const int32 CountInCluster : AgentsInClusters)
		{
			AgentsCounter += CountInCluster;
		}
		UpdatedEntitiesCountDelegate.Execute(AgentsCounter, OldAgentsCounter);
	}

	void AddAgentsCountInCluster(int32 ClusterType, int32 Count)
	{
		const int32 OldAgentsCounter = AgentsCounter;
		AgentsInClusters[ClusterType] += Count;
		AgentsCounter += Count;
		UpdatedEntitiesCountDelegate.Execute(AgentsCounter, OldAgentsCounter);
	}
	void RemoveAgentsCountInCluster(int32 ClusterType, int32 Count)
	{
		const int32 OldAgentsCounter = AgentsCounter;
		AgentsInClusters[ClusterType] -= Count;
		AgentsCounter -= Count;
		UpdatedEntitiesCountDelegate.Execute(AgentsCounter, OldAgentsCounter);
	}

	void UpdateAgentsCountersInAreas(const TArray<int32>& InAgentsInAreas)
	{
		AgentsInAreas = InAgentsInAreas;
	}
	void AddAgentsCountInArea(int32 AreaType, int32 Count)
	{
		AgentsInAreas[AreaType] += Count;
	}
	void RemoveAgentsCountInArea(int32 AreaType, int32 Count)
	{
		AgentsInAreas[AreaType] -= Count;
	}

	friend FArchive& operator <<(FArchive& Ar, FCrowdStatistics& Stats)
	{
		Ar << Stats.AgentsCounter;
		Ar << Stats.AgentsInClusters;
		Ar << Stats.CollisionsInClusters;
		Ar << Stats.ReachedFinishTimestamps;
		Ar << Stats.AverageEntitySpeedInClusters;
		return Ar;
	}

	int32 GetAgentsCount() const
	{
		return AgentsCounter;
	}
};


UCLASS()
class CLEVERCROWD_API UCrowdStatisticsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FCrowdStatistics Stats;
};
