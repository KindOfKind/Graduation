// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTypes.h"
#include "CollisionsFragments.generated.h"

USTRUCT()
struct CLEVERCROWD_API FCollisionFragment : public FMassFragment
{
	GENERATED_BODY()

	bool bCollidedAtPreviousTick = false;
	float LastCollisionCountTime = -100.f;

	int32 WeakCollisionsCounterMeta   = 0; // It's a test parameter that is managed from evaluator module
	int32 StrongCollisionsCounterMeta = 0; // It's a test parameter that is managed from evaluator module

	// To-the-side Avoidance---
	float AvoidingToSideTimeLeft = -1.f;
	bool bAvoidToTheRight        = false;

	// ORCA
	int32 OrcaIndex = -1;
};
