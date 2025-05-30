// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CCSPlayerPawn.generated.h"

class UFloatingPawnMovement;

UCLASS()
class CLEVERCROWDSIM_API ACCSPlayerPawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float HeightAboveFloor;

public:
	
	ACCSPlayerPawn();

protected:
	
	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	void AdjustFloorHeight();
};
