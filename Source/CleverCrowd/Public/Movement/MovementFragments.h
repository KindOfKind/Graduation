// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "MovementFragments.generated.h"

USTRUCT()
struct CLEVERCROWD_API FNavigationFragment : public FMassFragment
{
	GENERATED_BODY()

	int32 GoalPointIndex = 0;	// Index of a goal point this entity is moving to
	bool bCanUseDetour = true;	// Whether this entity can avoid dense crowd areas
};

USTRUCT()
struct CLEVERCROWD_API FMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	float CurrentSpeed = 0.f;	// cm/s
	UPROPERTY()
	FVector PrevTickLocation = FVector::ZeroVector;	// Used to calculate CurrentSpeed
};
