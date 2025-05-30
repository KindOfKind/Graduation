// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Flowfield/GoalPoint.h"
#include "ExitPoint.generated.h"

UCLASS()
class NAVIGATION_API AExitPoint : public AGoalPoint
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AExitPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual bool InteractWithEntity(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity) override;
};
