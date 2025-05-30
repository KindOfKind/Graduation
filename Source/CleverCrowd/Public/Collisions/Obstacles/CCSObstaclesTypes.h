// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"

struct CLEVERCROWD_API FCCSObstacleEdge
{
	FCCSObstacleEdge() = delete;
	FCCSObstacleEdge(const FVector& InStart, const FVector& InEnd)
	{
		Start   = InStart;
		End     = InEnd;
		LeftDir = FVector::CrossProduct((End - Start).GetSafeNormal(), FVector::UpVector);
	}

	FVector Start   = FVector::ZeroVector;
	FVector End     = FVector::ZeroVector;
	FVector LeftDir = FVector::ZeroVector;

	friend uint32 GetTypeHash(const FCCSObstacleEdge& Other)
	{
		uint32 Hash = GetTypeHash(Other.Start);
		Hash        = HashCombine(Hash, GetTypeHash(Other.End));
		Hash        = HashCombine(Hash, GetTypeHash(Other.LeftDir));
		return Hash;
	}

	static void ExtractEdgesFromBox(TArray<FCCSObstacleEdge>& OutEdges, const UBoxComponent* BoxComponent)
	{
		const FRotator BoxRotation	= BoxComponent->GetComponentRotation();
		const FVector BoxCenter		= BoxComponent->GetComponentLocation();
		const FVector BoxExtent		= BoxComponent->GetScaledBoxExtent();
	
		OutEdges.Emplace(FCCSObstacleEdge(BoxCenter + BoxRotation.RotateVector(FVector(-BoxExtent.X, -BoxExtent.Y, 0.f)), 
			BoxCenter + BoxRotation.RotateVector(FVector(BoxExtent.X, -BoxExtent.Y, 0.f))));

		OutEdges.Emplace(FCCSObstacleEdge(BoxCenter + BoxRotation.RotateVector(FVector(BoxExtent.X, -BoxExtent.Y, 0.f)), 
			BoxCenter + BoxRotation.RotateVector(FVector(BoxExtent.X, BoxExtent.Y, 0.f))));

		OutEdges.Emplace(FCCSObstacleEdge(BoxCenter + BoxRotation.RotateVector(FVector(BoxExtent.X, BoxExtent.Y, 0.f)), 
			BoxCenter + BoxRotation.RotateVector(FVector(-BoxExtent.X, BoxExtent.Y, 0.f))));

		OutEdges.Emplace(FCCSObstacleEdge(BoxCenter + BoxRotation.RotateVector(FVector(-BoxExtent.X, BoxExtent.Y, 0.f)), 
			BoxCenter + BoxRotation.RotateVector(FVector(-BoxExtent.X, -BoxExtent.Y, 0.f))));
	}
};
