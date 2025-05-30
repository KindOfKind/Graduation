// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grids/UtilsGridTypes.h"
#include "CrowdEvaluationHashGrid.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMadeGroupAreasSnapshotSignature)


struct FCrowdGroupAgentsStats
{
private:
	TMap<int32, int32> AgentsOfGroups;	// [AgentTypeId][NumOfAgents]
	int32 AgentsCounter = 0;

public:
	bool operator==(const FCrowdGroupAgentsStats& Other) const
	{
		if (AgentsCounter != Other.AgentsCounter)
		{
			return false;
		}
		for (auto& [AgentTypeId, OtherAgentsNum] : Other.AgentsOfGroups)
		{
			if (AgentsOfGroups[AgentTypeId] != OtherAgentsNum)
			{
				return false;
			}
		}
		return true;
	}

	friend FArchive& operator<<(FArchive& Ar, FCrowdGroupAgentsStats& Stats)
	{
		if (Ar.IsSaving())
		{
			int32 IdsNum = Stats.AgentsOfGroups.Num();
			Ar << Stats.AgentsCounter;
			Ar << IdsNum;
			for (auto& [AgentTypeId, AgentsNum] : Stats.AgentsOfGroups)
			{
				Ar << AgentTypeId;
				Ar << AgentsNum;
			}
		}
		else
		{
			int32 IdsNum = 0;
			Ar << Stats.AgentsCounter;
			Ar << IdsNum;
			int32 AgentTypeId, AgentsNum;
			for (int32 i = 0; i < IdsNum; i++)
			{
				Ar << AgentTypeId;
				Ar << AgentsNum;
				Stats.AgentsOfGroups.Add(AgentTypeId, AgentsNum);
			}
		}

		return Ar;
	}
	
	friend inline uint32 GetTypeHash(const FCrowdGroupAgentsStats& Stats)
	{
		uint32 Hash = GetTypeHash(Stats.AgentsCounter);
		for (auto& AgentsOfGroup : Stats.AgentsOfGroups)
		{
			Hash = HashCombine(Hash, AgentsOfGroup.Key);
			Hash = HashCombine(Hash, AgentsOfGroup.Value);
		}
		return Hash;
	}
	
	TMap<int32, int32>& Get() { return AgentsOfGroups; };
	const TMap<int32, int32>& Get() const { return AgentsOfGroups; };
	
	void AddAgent(const int32 CrowdGroupId)
	{
		AgentsOfGroups.FindOrAdd(CrowdGroupId) += 1;
		AgentsCounter += 1;
	}

	void AddAgentsFromGroup(const FCrowdGroupAgentsStats& Other)
	{
		for (const auto& [GroupId, NumInGroup] : Other.AgentsOfGroups)
		{
			AgentsOfGroups.FindOrAdd(GroupId) += NumInGroup;
			AgentsCounter += NumInGroup;
		}
	}

	float CalculateSimilarityWithGroup(const FCrowdGroupAgentsStats& Other)
	{
		int32 SimilarAgents = 0;
		for (const auto& [GroupId, NumInOtherGroup] : Other.AgentsOfGroups)
		{
			const int32 NumInGroup = AgentsOfGroups.FindOrAdd(GroupId);
			SimilarAgents += (FMath::Min(NumInGroup, NumInOtherGroup) * 2);
		}
		return (static_cast<float>(SimilarAgents) / (AgentsCounter + Other.AgentsCounter));
	}

	void Clear()
	{
		AgentsOfGroups.Empty();
		AgentsCounter = 0;
	}
	
	int32 GetTotalAgents() const { return AgentsCounter; };

	int32 GetAgentsOfGroup(const int32 CrowdGroupId) const
	{
		const int32* Count = AgentsOfGroups.Find(CrowdGroupId);
		if (!Count) return 0;
		return *Count;
	}

	bool IsEmpty() const { return AgentsCounter == 0; }

	void DivideAgentCounts(const float Divider)
	{
		for (auto& [GroupId, AgentsNum] : AgentsOfGroups)
		{
			AgentsNum /= Divider;
		}
		AgentsCounter /= Divider;
	}
};

struct FCrowdGroupCell
{
	FCrowdGroupAgentsStats AgentsOfGroups;
};

struct FCrowdGroupAreaSnapshot
{
	FGridBounds Bounds;
	FCrowdGroupAgentsStats AgentsOfGroups;	// Total amount of agents of different CrowdGroups in the area at the moment of the snapshot

	bool operator==(const FCrowdGroupAreaSnapshot& Other) const
	{
		return (Bounds == Other.Bounds && AgentsOfGroups == Other.AgentsOfGroups);
	}
	
	friend inline uint32 GetTypeHash(const FCrowdGroupAreaSnapshot& Snapshot)
	{
		return HashCombine(GetTypeHash(Snapshot.Bounds), GetTypeHash(Snapshot.AgentsOfGroups));
	}
};

struct FCrowdGroupAreaSnapshotsContainer
{
	TArray<FCrowdGroupAreaSnapshot> Snapshots;

	bool operator==(const FCrowdGroupAreaSnapshotsContainer& Other) const
	{
		return Snapshots == Other.Snapshots;
	}

	bool IsEmpty() const { return Snapshots.IsEmpty(); }

	void SetSnapshotAtIndex(const FCrowdGroupAreaSnapshot& Snapshot, int32 Index)
	{
		if (const int32 ElemsDifference = Index - Snapshots.Num() + 1; ElemsDifference > 0)
		{
			Snapshots.AddDefaulted(ElemsDifference);
		}
		Snapshots[Index] = Snapshot;
	}

	FCrowdGroupAreaSnapshot& GetLastSnapshot()
	{
		return Snapshots.Last();
	}

	static int32 GetSnapshotIndexAtTimeClamped(const float Time, const float SnapshotsRate)
	{
		int32 SnapshotIndex = (Time / SnapshotsRate) - 1;
		SnapshotIndex       = FMath::Max(SnapshotIndex, 0);
		return SnapshotIndex;
	}
};


UCLASS()
class EVALUATOR_API ACrowdEvaluationHashGrid : public AActor
{
	GENERATED_BODY()

	friend class UGameEvaluatorSubsystem;

public:
	static constexpr int32 MinCrowdGroupArea = 80;
	static constexpr int32 CellSize = 100;
	float SnapshotsRate = 1.f;

	FOnMadeGroupAreasSnapshotSignature OnMadeGroupAreasSnapshotDelegate;

protected:
	inline static int32 MaxCrowdGroupTypes = 30;
	
	UPROPERTY()
	UGameEvaluatorSubsystem* GameEvaluator;

private:
	TMap<FGridCellPosition, FCrowdGroupCell> CrowdGroupCells;
	// Index - CrowdGroup Id; Value - GroupArea snapshots at different moments.
	// It's a MultiMap because multiple groups with the same Id may exist at the same time in different map areas.
	TMultiMap<int32, FCrowdGroupAreaSnapshotsContainer> GroupAreaSnapshotsContainers;
	TArray<FCrowdGroupAgentsStats> AveragedGroupAreasStats;	// Index - CrowdGroup Id

	TMap<FGridCellPosition, int32> AreaIdCells;	// WIP. Used to effectively check Map Area Type at location. 

	bool bAllowNewGroupTypesCreation = true;

public:
	ACrowdEvaluationHashGrid();

protected:
	virtual void BeginPlay() override;

public:
	void AddAgentToCrowdGroupCell(const FGridCellPosition& CellPosition, const int32 CrowdGroupIdx);	// ToDo: replace CrowdGroupIdx with AgentTypeId
	void AddAgentToCrowdGroupCell(const FVector& Location, const int32 CrowdGroupIdx);	// ToDo: replace CrowdGroupIdx with AgentTypeId
	void ClearCrowdGroupCells();
	void MakeCrowdGroupAreasSnapshot();

	TMultiMap<int32, FCrowdGroupAreaSnapshotsContainer>* GetGroupAreaSnapshotsContainers() { return &GroupAreaSnapshotsContainers; }
	bool IsNewGroupTypesCreationAllowed() const { return bAllowNewGroupTypesCreation; }

	void DebugDrawCrowdGroupAreas(const UWorld* World, const float LifeTime, const float Thickness);
	void DebugDrawCrowdGroupAreasAveraged(const UWorld* World, const float LifeTime, const float Thickness);

	void SetAreaIdInCell(const FGridCellPosition& CellPosition, const int32 AreaId);
	int32 GetAreaIdAtLocation(const FVector& Location) const;

private:
	void MakeGroupAreaSnapshotFromCell(FCrowdGroupAreaSnapshot& OutSnapshot, const FGridCellPosition& InitCellPosition);
	// Merges the specified snapshot into another overlapping snapshot with overlapping Bounds (merges into a snapshot with the bigger bounds area)
	bool AssignGroupTypeAndTryMerge(FCrowdGroupAreaSnapshot& Snapshot);
	void SearchIntersectingGroupsOnSnapshots(int32& OutMaxIntersectingGroupId, int32& OutMaxIntersectionArea, FCrowdGroupAreaSnapshotsContainer*& OutSnapshotsContainer, FCrowdGroupAreaSnapshot& Snapshot,
	                                         const int32 SnapshotsIndex, bool bMerge);

	void RecalculateAveragedGroupAreasStats();
	void FindSimilarOrInitializeCrowdGroup(int32& OutNewGroupId, const FCrowdGroupAgentsStats& GroupAgentsStats);
	int32 GetFreeGroupTypeId() const;
	void ForEachNonEmptyGroupType(TFunction<void(int32)> Callback);

	FGridBounds GetAveragedCrowdGroupBounds(FCrowdGroupAreaSnapshotsContainer& SnapshotsContainer);
};
