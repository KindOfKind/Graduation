// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "CommonTypes.generated.h"

USTRUCT(BlueprintType)
struct FAggregatedValueFloat
{
	GENERATED_BODY()

	// Result of summing all aggregated values
	UPROPERTY(SaveGame)
	float TotalValue = 0.f;
	// Number of all summed values
	UPROPERTY(SaveGame)
	int32 ValuesAmount = 0;

	void AddValue(const float Value)
	{
		TotalValue += Value;
		ValuesAmount += 1;
	}

	float GetMean() const
	{
		if (ValuesAmount == 0)
		{
			return 0.0;
		}
		return TotalValue / ValuesAmount;
	}
	
	FAggregatedValueFloat& operator += (const FAggregatedValueFloat& Other)
	{
		TotalValue += Other.TotalValue;
		ValuesAmount += Other.ValuesAmount;
		return *this;
	}

	FAggregatedValueFloat operator + (const FAggregatedValueFloat& V2) const
	{
		FAggregatedValueFloat NewV;
		NewV.TotalValue   = TotalValue + V2.TotalValue;
		NewV.ValuesAmount = ValuesAmount + V2.ValuesAmount;
		return NewV;
	}

	friend FArchive& operator<<(FArchive& Ar, FAggregatedValueFloat& Value)
	{
		Ar << Value.TotalValue;
		Ar << Value.ValuesAmount;

		return Ar;
	}
};