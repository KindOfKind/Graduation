// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdEvaluationHashGrid.h"

#include "GameEvaluatorSubsystem.h"
#include "Grids/DebugBPFunctionLibrary.h"
#include "Grids/GridUtilsFunctionLibrary.h"


ACrowdEvaluationHashGrid::ACrowdEvaluationHashGrid()
{
	//GroupAreaSnapshotsContainers.AddDefaulted(MaxCrowdGroupTypes);
	AveragedGroupAreasStats.AddDefaulted(MaxCrowdGroupTypes);	// Temp solution. Consider we won't have more than MaxCrowdGroupTypes CrowdGroup Ids
}

void ACrowdEvaluationHashGrid::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACrowdEvaluationHashGrid::AddAgentToCrowdGroupCell(const FGridCellPosition& CellPosition, const int32 CrowdGroupIdx)
{
	CrowdGroupCells.FindOrAdd(CellPosition).AgentsOfGroups.AddAgent(CrowdGroupIdx);
}

void ACrowdEvaluationHashGrid::AddAgentToCrowdGroupCell(const FVector& Location, const int32 CrowdGroupIdx)
{
	const FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);;
	CrowdGroupCells.FindOrAdd(CellPosition).AgentsOfGroups.AddAgent(CrowdGroupIdx);
}

void ACrowdEvaluationHashGrid::ClearCrowdGroupCells()
{
	CrowdGroupCells.Reset();
}

void ACrowdEvaluationHashGrid::MakeCrowdGroupAreasSnapshot()
{
	if (bAllowNewGroupTypesCreation)
	{
		RecalculateAveragedGroupAreasStats();
	}
	
	while (!CrowdGroupCells.IsEmpty())
	{
		bool bFoundNonDefaultGroup = false;
		
		for (const auto& [CrowdGroupCell, CellData] : CrowdGroupCells)
		{
			if (CellData.AgentsOfGroups.GetTotalAgents() == CellData.AgentsOfGroups.GetAgentsOfGroup(0))
			{
				continue;
			}
			bFoundNonDefaultGroup = true;

			FCrowdGroupAreaSnapshot Snapshot;
			MakeGroupAreaSnapshotFromCell(Snapshot, CrowdGroupCell);

			if (Snapshot.Bounds.GetArea() < MinCrowdGroupArea)
			{
				break;
			}
			const bool bMergedWithOtherBounds = AssignGroupTypeAndTryMerge(Snapshot);
			
			break;
		}
		if (!bFoundNonDefaultGroup)
		{
			break;
		}
	}

	OnMadeGroupAreasSnapshotDelegate.Broadcast();
}

void ACrowdEvaluationHashGrid::MakeGroupAreaSnapshotFromCell(FCrowdGroupAreaSnapshot& OutSnapshot, const FGridCellPosition& InitCellPosition)
{
	OutSnapshot        = FCrowdGroupAreaSnapshot();
	OutSnapshot.Bounds = FGridBounds{InitCellPosition, InitCellPosition};

	TQueue<FGridCellPosition> CellsQueue;
	TSet<FGridCellPosition> VisitedCells;
	CellsQueue.Enqueue(InitCellPosition);
	VisitedCells.Add(InitCellPosition);

	FGridCellPosition Cell;
	while (CellsQueue.Dequeue(Cell))
	{
		// If a cell contains only agents of GroupIndex == 0, add the cell to statistics but ignore when updating bounds
		FCrowdGroupCell& GroupCell = CrowdGroupCells[Cell];
		if (GroupCell.AgentsOfGroups.GetTotalAgents() == GroupCell.AgentsOfGroups.GetAgentsOfGroup(0))
		{
			OutSnapshot.AgentsOfGroups.AddAgentsFromGroup(GroupCell.AgentsOfGroups);
			CrowdGroupCells.Remove(Cell);
			continue;
		}

		OutSnapshot.AgentsOfGroups.AddAgentsFromGroup(GroupCell.AgentsOfGroups);
		OutSnapshot.Bounds.UpdateToFitCell(Cell);
		CrowdGroupCells.Remove(Cell);

		TArray<FGridCellPosition> AdjacentCells;
		UGridUtilsFunctionLibrary::GetAdjacentCells4(AdjacentCells, Cell);
		for (const FGridCellPosition& AdjacentCell : AdjacentCells)
		{
			if (!CrowdGroupCells.Contains(AdjacentCell) || VisitedCells.Contains(AdjacentCell))
			{
				continue;
			}
			VisitedCells.Add(AdjacentCell);
			CellsQueue.Enqueue(AdjacentCell);
		}
	}
}

bool ACrowdEvaluationHashGrid::AssignGroupTypeAndTryMerge(FCrowdGroupAreaSnapshot& Snapshot)
{
	// ToDo: if merging, clear previous agents counter!!
	
	bool bMerged = false;

	int32 CurrentSnapshotIndex = FCrowdGroupAreaSnapshotsContainer::GetSnapshotIndexAtTimeClamped(GetWorld()->GetTimeSeconds(), SnapshotsRate);
	int32 NewGroupId = INDEX_NONE;
	FindSimilarOrInitializeCrowdGroup(NewGroupId, Snapshot.AgentsOfGroups);
	ensureMsgf(NewGroupId != INDEX_NONE, TEXT("Failed to assign any Crowd Group Type. You must form Crowd Group Types collection during the first test after launching the game."));
	
	// "Consume" intersected areas (assign currently identified Group Type to them)
	
	int32 MaxIntersectingGroupId = INDEX_NONE;
	int32 MaxIntersectionArea = 0;
	FCrowdGroupAreaSnapshotsContainer* IntersectedSnapshotsContainer;
	SearchIntersectingGroupsOnSnapshots(MaxIntersectingGroupId, MaxIntersectionArea, IntersectedSnapshotsContainer, Snapshot, CurrentSnapshotIndex, true);
	
	if (MaxIntersectingGroupId > INDEX_NONE)
	{
		FCrowdGroupAreaSnapshotsContainer TempSnapshotsContainer = *IntersectedSnapshotsContainer;
		Snapshot.AgentsOfGroups.AddAgentsFromGroup(IntersectedSnapshotsContainer->GetLastSnapshot().AgentsOfGroups);
		GroupAreaSnapshotsContainers.Remove(MaxIntersectingGroupId, *IntersectedSnapshotsContainer);
		GroupAreaSnapshotsContainers.Add(NewGroupId, TempSnapshotsContainer).SetSnapshotAtIndex(Snapshot, CurrentSnapshotIndex);
		bMerged = true;
	}
	else
	{
		int32 PreviousSnapshotIndex = CurrentSnapshotIndex - 1;
		if (PreviousSnapshotIndex >= 0)
		{
			SearchIntersectingGroupsOnSnapshots(MaxIntersectingGroupId, MaxIntersectionArea, IntersectedSnapshotsContainer, Snapshot, PreviousSnapshotIndex, false);
		}
		if (MaxIntersectingGroupId > INDEX_NONE)	// If found some group at the same place on the previous snapshot, take this group's id
		{
			FCrowdGroupAreaSnapshotsContainer TempSnapshotsContainer = *IntersectedSnapshotsContainer;
			GroupAreaSnapshotsContainers.Remove(MaxIntersectingGroupId, *IntersectedSnapshotsContainer);
			GroupAreaSnapshotsContainers.Add(NewGroupId, TempSnapshotsContainer).SetSnapshotAtIndex(Snapshot, CurrentSnapshotIndex);
			bMerged = true;
		}
		else	// Add new group type
		{
			GroupAreaSnapshotsContainers.Add(NewGroupId).SetSnapshotAtIndex(Snapshot, CurrentSnapshotIndex);
		}
	}

	return bMerged;
}

void ACrowdEvaluationHashGrid::SearchIntersectingGroupsOnSnapshots(int32& OutMaxIntersectingGroupId, int32& OutMaxIntersectionArea, FCrowdGroupAreaSnapshotsContainer*& OutSnapshotsContainer,
                                                                   FCrowdGroupAreaSnapshot& Snapshot, const int32 SnapshotsIndex, bool bMerge)
{
	constexpr int32 MinIntersectionArea = 30;
	OutMaxIntersectingGroupId = INDEX_NONE;
	OutMaxIntersectionArea    = 0;
	FGridBounds MaxIntersectedBounds;

	ForEachNonEmptyGroupType([&](int32 GroupId)
	{
		TArray<FCrowdGroupAreaSnapshotsContainer*> GroupSnapshots;
		GroupAreaSnapshotsContainers.MultiFindPointer(GroupId, GroupSnapshots);
		for (FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer : GroupSnapshots)
		{
			if (SnapshotsContainer->IsEmpty() || !SnapshotsContainer->Snapshots.IsValidIndex(SnapshotsIndex))
			{
				continue;
			}
		
			FGridBounds& OtherBounds = SnapshotsContainer->Snapshots[SnapshotsIndex].Bounds;
			const int32 IntersectionArea = Snapshot.Bounds.GetIntersectionArea(OtherBounds);
			if (IntersectionArea > MinIntersectionArea && IntersectionArea > OutMaxIntersectionArea)
			{
				OutMaxIntersectionArea    = IntersectionArea;
				OutMaxIntersectingGroupId = GroupId;
				OutSnapshotsContainer     = SnapshotsContainer;
				MaxIntersectedBounds      = SnapshotsContainer->Snapshots[SnapshotsIndex].Bounds;
			}
		}
	});

	if (bMerge && OutMaxIntersectingGroupId > INDEX_NONE)
	{
		Snapshot.Bounds.MergeWithBounds(MaxIntersectedBounds);
	}
}

void ACrowdEvaluationHashGrid::RecalculateAveragedGroupAreasStats()
{
	ForEachNonEmptyGroupType([&](int32 GroupId)
	{
		TArray<FCrowdGroupAreaSnapshotsContainer*> GroupSnapshots;
		GroupAreaSnapshotsContainers.MultiFindPointer(GroupId, GroupSnapshots);
		// If multiple groups with the same GroupId are present, we calculate average by the first of them
		for (int32 i = GroupSnapshots.Num() - 1; i >= 0; i--)
		{
			FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer = GroupSnapshots[i];
			if (SnapshotsContainer->IsEmpty())
			{
				continue;
			}

			FCrowdGroupAgentsStats AveragedStats;
			int32 SnapshotsNum = SnapshotsContainer->Snapshots.Num();
			for (FCrowdGroupAreaSnapshot& Snapshot : SnapshotsContainer->Snapshots)
			{
				AveragedStats.AddAgentsFromGroup(Snapshot.AgentsOfGroups);
			}
			AveragedStats.DivideAgentCounts(SnapshotsNum);

			AveragedGroupAreasStats[GroupId] = AveragedStats;
			break;
		}
	});

	GameEvaluator->AveragedGroupAreasStatsCached = AveragedGroupAreasStats;
}
UE_DISABLE_OPTIMIZATION
void ACrowdEvaluationHashGrid::FindSimilarOrInitializeCrowdGroup(int32& OutNewGroupId, const FCrowdGroupAgentsStats& GroupAgentsStats)
{
	constexpr float SimilarityThreshold = 0.7f;	// If similarity is less than this threshold, a new CrowdGroup type will be created

	const int32 AgentTypesNum  = GroupAgentsStats.Get().Num();
	OutNewGroupId              = INDEX_NONE;
	float MaxSimilarity        = 0.f;
 	int32 MaxSimilarityGroupId = INDEX_NONE;
	
	for (int32 GroupId = 0; GroupId < AveragedGroupAreasStats.Num(); GroupId++)
	{
		FCrowdGroupAgentsStats OtherStats = AveragedGroupAreasStats[GroupId];
		if (OtherStats.IsEmpty()) continue;

		// Use this version of similarity calculation if you want significance of a specific agent type to vary based on how many agents of this type is in the crowd group
		// FCrowdGroupAgentsStats StatsDiff  = GroupAgentsStats;
		// StatsDiff.DivideAgentCounts(static_cast<float>(GroupAgentsStats.GetTotalAgents()) / OtherStats.GetTotalAgents());
		// const float Similarity = StatsDiff.CalculateSimilarityWithGroup(OtherStats);

		float SimilaritiesSum = 0.f;
		for (auto& [AgentTypeId, AgentsOfType] : GroupAgentsStats.Get())
		{
			const int32 OtherAgentsOfType        = OtherStats.GetAgentsOfGroup(AgentTypeId);
			const int32 MaxAgentsOfType          = FMath::Max(AgentsOfType, OtherAgentsOfType);
			const float AgentsOfTypePercent      = FMath::Max(0.01f, static_cast<float>(AgentsOfType) / MaxAgentsOfType);
			const float OtherAgentsOfTypePercent = FMath::Max(0.01f, static_cast<float>(OtherAgentsOfType) / MaxAgentsOfType);
			SimilaritiesSum += 1.f - abs(AgentsOfTypePercent - OtherAgentsOfTypePercent);
		}
		float Similarity = SimilaritiesSum / GroupAgentsStats.Get().Num();	// Average of similarities of all params

		if (MaxSimilarity < Similarity)
		{
			MaxSimilarity = Similarity;
			MaxSimilarityGroupId = GroupId;
		}
	}

	if (MaxSimilarity < SimilarityThreshold && bAllowNewGroupTypesCreation)	// Register a new group type if no similar existing groups
	{
		OutNewGroupId = GetFreeGroupTypeId();
	}
	else	// Return the most similar group
	{
		OutNewGroupId = MaxSimilarityGroupId;
	}
}

int32 ACrowdEvaluationHashGrid::GetFreeGroupTypeId() const
{
	for (int32 GroupId = 0; GroupId < MaxCrowdGroupTypes; GroupId++)
	{
		const FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer = GroupAreaSnapshotsContainers.Find(GroupId);
		if (SnapshotsContainer == nullptr || SnapshotsContainer->IsEmpty())
		{
			return GroupId;
		}
	}
	return INDEX_NONE;
}

void ACrowdEvaluationHashGrid::ForEachNonEmptyGroupType(TFunction<void(int32)> Callback)
{
	for (int32 GroupId = 0; GroupId < MaxCrowdGroupTypes; GroupId++)
	{
		const FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer = GroupAreaSnapshotsContainers.Find(GroupId);
		if (SnapshotsContainer && !SnapshotsContainer->IsEmpty())
		{
			Callback(GroupId);
		}
	}
}

UE_ENABLE_OPTIMIZATION

FGridBounds ACrowdEvaluationHashGrid::GetAveragedCrowdGroupBounds(FCrowdGroupAreaSnapshotsContainer& SnapshotsContainer)
{
	FGridBounds AveragedBounds;
	int32 Counter = 0;
	for (FCrowdGroupAreaSnapshot& Snapshot : SnapshotsContainer.Snapshots)
	{
		if (Snapshot.Bounds.GetArea() < 2)
		{
			continue;
		}

		if (AveragedBounds.GetArea() < 2)
		{
			AveragedBounds = Snapshot.Bounds;
			Counter += 1;
			continue;
		}
		AveragedBounds.BottomLeftCell.X += Snapshot.Bounds.BottomLeftCell.X;
		AveragedBounds.BottomLeftCell.Y += Snapshot.Bounds.BottomLeftCell.Y;
		AveragedBounds.TopRightCell.X   += Snapshot.Bounds.TopRightCell.X;
		AveragedBounds.TopRightCell.Y   += Snapshot.Bounds.TopRightCell.Y;
		Counter += 1;
	}
	if (Counter <= 0)
	{
		return AveragedBounds;
	}
	AveragedBounds.BottomLeftCell.X /= Counter;
	AveragedBounds.BottomLeftCell.Y /= Counter;
	AveragedBounds.TopRightCell.X   /= Counter;
	AveragedBounds.TopRightCell.Y   /= Counter;
	return AveragedBounds;
}


void ACrowdEvaluationHashGrid::DebugDrawCrowdGroupAreas(const UWorld* World, const float LifeTime, const float Thickness)
{
	const int32 CurrentSnapshotIndex = FCrowdGroupAreaSnapshotsContainer::GetSnapshotIndexAtTimeClamped(GetWorld()->GetTimeSeconds(), SnapshotsRate);
	ForEachNonEmptyGroupType([&](int32 GroupId)
	{
		TArray<FCrowdGroupAreaSnapshotsContainer*> GroupSnapshots;
		GroupAreaSnapshotsContainers.MultiFindPointer(GroupId, GroupSnapshots);
		for (FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer : GroupSnapshots)
		{
			if (!SnapshotsContainer->Snapshots.IsValidIndex(CurrentSnapshotIndex)) continue;
			
			const FGridBounds& Bounds = SnapshotsContainer->Snapshots[CurrentSnapshotIndex].Bounds;
			if (Bounds.GetArea() < 2) continue;
		
			FVector Extent = Bounds.GetExtent(static_cast<float>(CellSize));
			Extent.Z += 500.f;
			FVector Center = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(Bounds.GetCenterCell(), CellSize);
			FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(GroupId);
			DrawDebugBox(World, Center, Extent, DebugColor, false, LifeTime, 0, Thickness);
		}
	});
}

void ACrowdEvaluationHashGrid::DebugDrawCrowdGroupAreasAveraged(const UWorld* World, const float LifeTime, const float Thickness)
{
	int32 Test = 0;
	ForEachNonEmptyGroupType([&](int32 GroupId)
	{
		TArray<FCrowdGroupAreaSnapshotsContainer*> GroupSnapshots;
		GroupAreaSnapshotsContainers.MultiFindPointer(GroupId, GroupSnapshots);
		for (FCrowdGroupAreaSnapshotsContainer* SnapshotsContainer : GroupSnapshots)
		{
			FGridBounds Bounds = GetAveragedCrowdGroupBounds(*SnapshotsContainer);
			if (Bounds.GetArea() < 2)
			{
				continue;
			}

			// Uncomment to exclude bounds that intersect other bigger bounds
			// bool bIntersectsBiggerBounds = false;
			// for (int32 OtherGroupId = 0; OtherGroupId < GroupTypesNum; OtherGroupId++)
			// {
			// 	TArray<FCrowdGroupAreaSnapshotsContainer*> OtherGroupSnapshots;
			// 	GroupAreaSnapshotsContainers.MultiFindPointer(OtherGroupId, OtherGroupSnapshots);
			// 	for (FCrowdGroupAreaSnapshotsContainer* OtherSnapshotsContainer : GroupSnapshots)
			// 	{
			// 		FGridBounds OtherBounds = GetAveragedCrowdGroupBounds(*OtherSnapshotsContainer);
			// 		if (OtherBounds.GetArea() < 2)
			// 		{
			// 			continue;
			// 		}
			// 		if (Bounds.GetIntersectionArea(OtherBounds) > 10 && OtherBounds.GetArea() > Bounds.GetArea())
			// 		{
			// 			bIntersectsBiggerBounds = true;
			// 			break;
			// 		}
			// 	}
			// 	if (bIntersectsBiggerBounds == true)
			// 	{
			// 		break;
			// 	}
			// }
			// if (bIntersectsBiggerBounds)
			// {
			// 	continue;
			// }
		
			FVector Extent = Bounds.GetExtent(static_cast<float>(CellSize));
			Extent.Z += 900.f;
			FVector Center = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(Bounds.GetCenterCell(), CellSize);
			FColor DebugColor = UDebugBPFunctionLibrary::GetColorByID(GroupId);
			DrawDebugBox(World, Center, Extent, DebugColor, false, LifeTime, 0, Thickness);
		}
	});
}

void ACrowdEvaluationHashGrid::SetAreaIdInCell(const FGridCellPosition& CellPosition, const int32 AreaId)
{
	AreaIdCells.Add(CellPosition, AreaId);
}

int32 ACrowdEvaluationHashGrid::GetAreaIdAtLocation(const FVector& Location) const
{
	FGridCellPosition CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(Location, CellSize);
	const int32* AreaId = AreaIdCells.Find(CellPosition);
	if (AreaId)
	{
		return *AreaId;
	}
	return -1;
}
