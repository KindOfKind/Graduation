// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowfieldManager.generated.h"

class AFlowfield;

UCLASS()
class NAVIGATION_API AFlowfieldManager : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TObjectPtr<AFlowfield> Flowfield;

public:
	
	AFlowfieldManager();

protected:
	
	virtual void BeginPlay() override;

public:

	AFlowfield* InitializeFlowfield();
	AFlowfield* GetFlowfield();
	
};
