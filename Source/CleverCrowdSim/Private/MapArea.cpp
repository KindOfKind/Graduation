// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/MapArea.h"

#include "Grids/GridUtilsFunctionLibrary.h"


AMapArea::AMapArea()
{
	PrimaryActorTick.bCanEverTick = false;
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
}

void AMapArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FGridCellPosition ActorCell = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(GetActorLocation(), CellSize);
	OffsetBounds.BottomLeftCell       = Bounds.BottomLeftCell + ActorCell;
	OffsetBounds.TopRightCell         = Bounds.TopRightCell + ActorCell;
	
	DrawDebugBounds();
}

void AMapArea::BeginPlay()
{
	Super::BeginPlay();
}

FGridBounds AMapArea::GetBounds()
{
	if (OffsetBounds == FGridBounds())
	{
		const FGridCellPosition ActorCell = UGridUtilsFunctionLibrary::GetGridCellPositionAtLocation(GetActorLocation(), CellSize);
		OffsetBounds.BottomLeftCell       = Bounds.BottomLeftCell + ActorCell;
		OffsetBounds.TopRightCell         = Bounds.TopRightCell + ActorCell;
	}
	return OffsetBounds;
}

void AMapArea::DrawDebugBounds(float LifeTime, float Thickness)
{
	FVector Extent = OffsetBounds.GetExtent(static_cast<float>(CellSize));
	Extent.Z += 500.f;
	FVector Center = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(OffsetBounds.GetCenterCell(), CellSize);
	DrawDebugBox(GetWorld(), Center, Extent, FColor::Red, false, LifeTime, 0, Thickness);
}
