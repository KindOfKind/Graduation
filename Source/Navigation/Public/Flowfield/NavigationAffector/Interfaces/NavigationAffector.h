// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NavigationAffector.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UNavigationAffector : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NAVIGATION_API INavigationAffector
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	uint8 GetFlowfieldNavigationCost();
};
