// Fill out your copyright notice in the Description page of Project Settings.


#include "CleverCrowdSim/Public/GameManagement/CCSGameMode.h"

#include "EngineUtils.h"
#include "GameEvaluatorSubsystem.h"
#include "Entity/EntityNotifierSubsystem.h"
#include "Flowfield/Flowfield.h"
#include "Flowfield/GoalPoint.h"
#include "GameManagement/GCCGameInstance.h"
#include "Global/CrowdNavigationSubsystem.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "Management/CCSEntitiesManagerSubsystem.h"
#include "Management/CrowdNavigatorSubsystem.h"
#include "MapAnalyzer/MapAnalyzerSubsystem.h"
#include "MapAnalyzer/ClusterDefiners/SimpleMapClusterDefiner.h"

void ACCSGameMode::BeginPlay()
{
	Super::BeginPlay();

	check(GetWorld());
	GCCGameInstance = Cast<UGCCGameInstance>(GetGameInstance());
	GCCGameInstance->LoadMapAreasConfigsFromFile();
	
	GameEvaluator            = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	CrowdStatisticsSubsystem = GetWorld()->GetSubsystem<UCrowdStatisticsSubsystem>();
	EntitiesManagerSubsystem = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>();
	CrowdNavigator           = GetWorld()->GetSubsystem<UCrowdNavigatorSubsystem>();
	CrowdNavigationSubsystem = GetWorld()->GetSubsystem<UCrowdNavigationSubsystem>();
	CrowdNavigationSubsystem->InitializeNavigation();

	CollisionsSubsystem = GetWorld()->GetSubsystem<UCCSCollisionsSubsystem>();
	CollisionsSubsystem->InitializeObstaclesOnMap();

	MapAnalyzerSubsystem                           = GetWorld()->GetSubsystem<UMapAnalyzerSubsystem>();
	USimpleMapClusterDefiner* ClusterDefiner       = NewObject<USimpleMapClusterDefiner>();
	const FClusterizationRules ClusterizationRules = FClusterizationRules(4, ClusterDefiner);
	MapAnalyzerSubsystem->MapAreasDataConfig       = GCCGameInstance->MapAreasDataConfigCached;
	MapAnalyzerSubsystem->InitializeOuter();
	MapAnalyzerSubsystem->SetMapClusterizationRules(ClusterizationRules);
	MapAnalyzerSubsystem->DefineClustersOnMap();	// ToDo: Remove it from here after tests.
	MapAnalyzerSubsystem->DefineMapAreasOnMap();

	GameEvaluator->MaxAreaTypeOnLevel = MapAnalyzerSubsystem->MaxMapAreaTypeId;
	GameEvaluator->OnBeginPlay();
	
	// ToDo: move into a separate method UpdateMapAreasDataConfig.
	// Basically this check means that we're in "evaluator learning" phase. We want to modify MapAreasDataConfig only during this phase.
	if (GameEvaluator->GetEvaluationHashGrid()->IsNewGroupTypesCreationAllowed())
	{
		GameEvaluator->GetEvaluationHashGrid()->OnMadeGroupAreasSnapshotDelegate.AddLambda([this]()
		{
			MapAnalyzerSubsystem->MapAreasDataConfig.Empty();

			TMultiMap<int32, FCrowdGroupAreaSnapshotsContainer>* GroupAreaSnapshotsContainers = GameEvaluator->GetEvaluationHashGrid()->GetGroupAreaSnapshotsContainers();
			if (!GroupAreaSnapshotsContainers) return;
			for (auto& [CrowdGroupId, SnapshotsContainer] : *GroupAreaSnapshotsContainers)
			{
				if (SnapshotsContainer.IsEmpty())
				{
					continue;
				}

				FMapAreaData AreaData;
				MapAnalyzerSubsystem->EvaluateMapAreaDataInBounds(AreaData, SnapshotsContainer.GetLastSnapshot().Bounds);
				MapAnalyzerSubsystem->MapAreasDataConfig.Add(CrowdGroupId, AreaData);

				GCCGameInstance->MapAreasDataConfigCached = MapAnalyzerSubsystem->MapAreasDataConfig;
			}
		});
	}
	
	EntityNotifier = GetWorld()->GetSubsystem<UEntityNotifierSubsystem>();
	CrowdStatisticsSubsystem->Stats.UpdatedEntitiesCountDelegate.BindLambda([this](int32 NewCount, int32 OldCount)
	{
		GameEvaluator->OnUpdatedEntitiesCount(NewCount, OldCount);
	});

	bDisableDetour = GameEvaluator->IsEvaluatingMapAreas();	// The Detour is disabled when evaluating map areas to not effect the results
	if (!bDisableDetour)
	{
		InitDetoursSearcher();
	}

	EntitiesManagerSubsystem->SpawnAllAgents();
}

void ACCSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GameEvaluator->IsPendingTestFinish())
	{
		GameEvaluator->SaveTestData();
		GameEvaluator->TransitionToRandomTestLevel();
		return;
	}
	
	GameEvaluator->OnTick(GetWorld()->DeltaRealTimeSeconds);	// Delta seconds that are adjusted for slomo
}

void ACCSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DetourSearcherRunnable)
	{
		DetourSearcherRunnable->CalculatedDetoursDelegate.Unbind();
	}

	GetWorld()->GetTimerManager().ClearTimer(DetourRecalculationTh);

	GCCGameInstance->SaveMapAreasConfigsToFile();
	
	Super::EndPlay(EndPlayReason);
}

AFlowfield* ACCSGameMode::GetFlowfield()
{
	if (!IsValid(CrowdNavigationSubsystem))
	{
		return nullptr;
	}
	return CrowdNavigationSubsystem->GetFlowfield();
}

void ACCSGameMode::InitDetoursSearcher()
{
	check(IsValid(CrowdNavigator));
	DetourSearcherRunnable = new FDetourSearcherRunnable();
	DetourSearcherRunnable->CalculatedDetoursDelegate.BindUObject(this, &ACCSGameMode::ApplyDetourDirectionsGrids);
	CrowdNavigator->CacheDetourSearcher(DetourSearcherRunnable);
	DetourSearcherRunnable->MaxAdditionalCost = GameEvaluator->GetMetaParams().DetourMaxAdditionalCost.Value;
	
	FDetourSearcherRunnable::FPayload DetourPayload;
	DetourPayload.CostsGrid = *CrowdNavigationSubsystem->GetFlowfield()->CostsGrid.Get();
	DetourPayload.CollisionsGrid = CollisionsSubsystem->GetCollisionsHashGrid();

	TArray<FDetourSearcherRunnable::GoalInfo> GoalsInfos;
	UWorld* World = GetWorld();
	for (TActorIterator<AGoalPoint> It(World); It; ++It)
	{
		AGoalPoint* GoalPoint = *It;
		if (!GoalPoint) continue;

		int32 CellSize = DetourPayload.CollisionsGrid.GetCellSize();
		FDetourSearcherRunnable::GoalInfo GoalInfo;
		GoalInfo.CellPosition = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(GoalPoint->GetActorLocation(), CellSize);
		GoalInfo.GridSizes    = GoalPoint->GetGridSizes();
		GoalsInfos.Add(GoalInfo);
	}
	DetourPayload.GoalsInfos = GoalsInfos;
	
	DetourSearcherRunnable->StartCalculation(DetourPayload);
}

void ACCSGameMode::ApplyDetourDirectionsGrids(TArray<TSharedPtr<FDirectionsGrid>> DetourDirectionsGrids, TArray<FGridBounds> DenseAreas)
{
	if (!GetWorld() || !IsValid(CrowdNavigationSubsystem) || !IsValid(CrowdNavigationSubsystem->GetFlowfield()))
	{
		return;
	}

	DetourSearcherRunnable->CalculatedDetoursDelegate.Unbind();
	DetourSearcherRunnable = nullptr;
	CrowdNavigator->CacheDetourSearcher(nullptr);

	TArray<TSharedPtr<FDirectionsGrid>>& DirGrids = CrowdNavigationSubsystem->GetFlowfield()->DirectionsGrids;
	for (int32 i = 0; i < DirGrids.Num(); i++)
	{
		DirGrids[i].Get()->DetourDirectionsGrid = DetourDirectionsGrids[i];
	}

	// Enable detour for all entities. Then disable detour for entities inside DenseAreas. We don't want entities to go out of a dense area if they are already in the area. 
	EntitiesManagerSubsystem->SetDetourEnabledForAllEntities(true);
	for (FGridBounds DenseArea : DenseAreas)
	{
		if (DenseArea.TopRightCell.X - DenseArea.BottomLeftCell.X > 4)
		{
			DenseArea.BottomLeftCell.X += 1;	// To let agents on borders go apart from the group
			DenseArea.TopRightCell.X -= 1;
		}
		if (DenseArea.TopRightCell.Y - DenseArea.BottomLeftCell.Y > 4)
		{
			DenseArea.BottomLeftCell.Y += 1;
			DenseArea.TopRightCell.Y -= 1;
		}
		
		EntitiesManagerSubsystem->SetDetourEnabledForEntitiesInBounds(DenseArea, false);
	}

	// Calculate Detours again with delay
	GetWorld()->GetTimerManager().SetTimer(DetourRecalculationTh, [this]()
	{
		if (!IsValid(this)) return;
		InitDetoursSearcher();
	},
	DetoursSearchRate, false);
}
