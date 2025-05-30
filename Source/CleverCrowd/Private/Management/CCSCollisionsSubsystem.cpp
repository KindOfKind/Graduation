// Fill out your copyright notice in the Description page of Project Settings.


#include "Management/CCSCollisionsSubsystem.h"

#include "EngineUtils.h"
#include "Collisions/Obstacles/CCSObstaclesHashGrid.h"
#include "Collisions/Obstacles/CCSSimpleObstacle.h"
#include "Global/CleverCrowdGlobals.h"
#include "Grids/GridUtilsFunctionLibrary.h"

void UCCSCollisionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ObstaclesHashGrid = NewObject<UCCSObstaclesHashGrid>(this);
}

void UCCSCollisionsSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(CollisionsHashGrid.DecrementCollisionsCountTh);
	GetWorld()->GetTimerManager().ClearTimer(DebugCollisionsCountTh);
	
	Super::Deinitialize();
}

void UCCSCollisionsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	GetWorld()->GetTimerManager().SetTimer(CollisionsHashGrid.DecrementCollisionsCountTh, [this]()
	{
		if (!IsValid(this)) return;
		CollisionsHashGrid.DecrementCollisionsCountAtAllCells();
	},
	CollisionsHashGrid.DecrementCollisionsCountRate, true);
}

void UCCSCollisionsSubsystem::InitializeObstaclesOnMap()
{
	UWorld* World = GetWorld();
	for (TActorIterator<ACCSSimpleObstacle> It(World); It; ++It)
	{
		ACCSSimpleObstacle* Obstacle = *It;
		if (!Obstacle) continue;

		TArray<FCCSObstacleEdge> Edges;
		FCCSObstacleEdge::ExtractEdgesFromBox(Edges, Obstacle->GetBoxComp());
		ObstaclesHashGrid->AddObstacleEdges(Edges);
	}

	if (UE::CleverCrowd::Globals::bDrawDebugCollisionsCountsPeriodically)
	{
		DrawDebugCollisionsCountsPeriodically();
	}
}

void UCCSCollisionsSubsystem::DrawDebugCollisionsCountsPeriodically()
{
	GetWorld()->GetTimerManager().SetTimer(DebugCollisionsCountTh, [this]()
	{
		if (!IsValid(this) || !GetWorld()) return;
		CollisionsHashGrid.DrawDebugDenseAreas(GetWorld(), CollisionsHashGrid.DecrementCollisionsCountRate - 0.1f, 8);
		
		for (const auto& [CellPos, Count] : CollisionsHashGrid.GetCollisionsInCells())
		{
			const FVector Loc         = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(CellPos, 100);
			float DebugLineLengthMult = FMath::GetMappedRangeValueClamped(
				FVector2D{0.f, static_cast<double>(CollisionsHashGrid.MaxCollisionsCount)}, FVector2D{0.05f, 1.f}, Count);

			float ColorHue = FMath::GetMappedRangeValueClamped(
				FVector2D{0.f, static_cast<double>(CollisionsHashGrid.MaxCollisionsCount)}, FVector2D{140.f, 0.f}, Count);
			FColor HeatmappedColor = FColor{FLinearColor::MakeFromHSV8(ColorHue, 255, 255).ToFColor(true)};

			DrawDebugLine(GetWorld(), Loc, Loc + FVector::UpVector * DebugLineLengthMult * 500.f, HeatmappedColor, false,
			              CollisionsHashGrid.DecrementCollisionsCountRate, 0, 10.f);
		}
	}, CollisionsHashGrid.DecrementCollisionsCountRate, true);
}
