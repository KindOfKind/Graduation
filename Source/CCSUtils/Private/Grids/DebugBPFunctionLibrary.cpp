// Fill out your copyright notice in the Description page of Project Settings.


#include "Grids/DebugBPFunctionLibrary.h"

FColor UDebugBPFunctionLibrary::GetColorByID(const int32 ID)
{
	FColor DebugColor = FColor::Red;
	switch (ID % 8)
	{
	case 0:
		DebugColor = FColor::Red;
		break;
	case 1:
		DebugColor = FColor::Green;
		break;
	case 2:
		DebugColor = FColor::Blue;
		break;
	case 3:
		DebugColor = FColor::Yellow;
		break;
	case 4:
		DebugColor = FColor::Emerald;
		break;
	case 5:
		DebugColor = FColor::Purple;
		break;
	case 6:
		DebugColor = FColor::Cyan;
		break;
	default:
		DebugColor = FColor::Black;
		break;
	}

	return DebugColor;
}
