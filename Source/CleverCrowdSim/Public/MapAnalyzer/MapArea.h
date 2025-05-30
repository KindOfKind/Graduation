// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grids/UtilsGridTypes.h"
#include "MapArea.generated.h"

UCLASS()
class CLEVERCROWDSIM_API AMapArea : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> RootSceneComponent;

public:
	int32 CellSize = 100;

	UPROPERTY(EditAnywhere, Category = "Config")
	FGridBounds Bounds;

private:
	FGridBounds OffsetBounds;

public:
	AMapArea();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

public:
	// Offsets bounds at actor location
	FGridBounds GetBounds();
	
	void DrawDebugBounds(float LifeTime = 0.5f, float Thickness = 5.f);
};
