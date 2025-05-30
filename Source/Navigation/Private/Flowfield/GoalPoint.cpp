// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/GoalPoint.h"


AGoalPoint::AGoalPoint()
{
	EntityInteractionRange = 200.f;
	GridSizes = FGridSizes{0, 0};
}

void AGoalPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AGoalPoint::InteractWithEntity(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity)
{
	return true;
}
