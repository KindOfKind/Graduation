// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdNavigationSubsystem.generated.h"

class AFlowfield;
class AFlowfieldManager;
/**
 * 
 */
UCLASS()
class NAVIGATION_API UCrowdNavigationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:

	bool bNavigationInitialized;

	TObjectPtr<AFlowfieldManager> FlowfieldManager;

public:

	UCrowdNavigationSubsystem();
	
public:

	// Method that initializes all required navigation systems, e.g. spawns a valid flowfield.
	void InitializeNavigation();
	
	AFlowfieldManager* GetFlowfieldManager();
	AFlowfield* GetFlowfield();

private:

	// FLOWFIELD INITIALIZATION
	
	void InitializeFlowfieldManager();
	void InitializeFlowfield();

};
