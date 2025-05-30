// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DebugBPFunctionLibrary.generated.h"


UCLASS()
class CCSUTILS_API UDebugBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Debug")
	static FColor GetColorByID(const int32 ID);
};
