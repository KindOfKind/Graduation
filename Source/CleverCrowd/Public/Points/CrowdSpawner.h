// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "CrowdSpawner.generated.h"

UCLASS(Blueprintable)
class CLEVERCROWD_API ACrowdSpawner : public AMassSpawner
{
	GENERATED_BODY()

public:
	ACrowdSpawner();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
};
