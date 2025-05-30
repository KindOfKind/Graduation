// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCSSimpleObstacle.generated.h"

class UBoxComponent;

UCLASS()
class CLEVERCROWD_API ACCSSimpleObstacle : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> RootSceneComp;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> BoxComp;

public:
	ACCSSimpleObstacle();

protected:
	virtual void BeginPlay() override;

public:
	const UBoxComponent* GetBoxComp() { return BoxComp; }

};
