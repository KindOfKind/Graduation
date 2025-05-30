// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EntityNotifierSubsystem.generated.h"

struct FMassEntityHandle;

DECLARE_DELEGATE_OneParam(FEntityActionSignature, const FMassEntityHandle& Entity)

/**
 * Subsystem that notifies different modules about various actions over entities.
 */
UCLASS()
class CCSUTILS_API UEntityNotifierSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	FEntityActionSignature PreDestroyEntityDelegate;
};
