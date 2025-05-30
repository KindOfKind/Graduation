// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridTypes.h"
#include "GameFramework/Actor.h"
#include "GoalPoint.generated.h"

struct FMassEntityHandle;
struct FMassEntityManager;
struct FDirectionsGrid;
class AFlowfield;

UCLASS(Blueprintable)
class NAVIGATION_API AGoalPoint : public AActor
{
	GENERATED_BODY()

	friend class UFlowfieldCalculatorComponent;

public:
	float EntityInteractionRange;

protected:
	TWeakPtr<FDirectionsGrid> OwningDirectionsGrid;

private:
	UPROPERTY(EditAnywhere, Category="Config", meta = (AllowPrivateAccess = "true"))
	FGridSizes GridSizes;
	
public:
	AGoalPoint();

protected:
	virtual void BeginPlay() override;

public:
	FGridSizes GetGridSizes() { return GridSizes; };

	virtual bool InteractWithEntity(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity);
};
