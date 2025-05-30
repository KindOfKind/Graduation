// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/NavigationAffector.h"
#include "SimpleNavigationAffector.generated.h"


UCLASS()
class NAVIGATION_API ASimpleNavigationAffector : public AActor, public INavigationAffector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

private:

	UPROPERTY(EditAnywhere, Category = "Config", meta = (AllowPrivateAccess = "true"))
	uint8 NavigationCost;

public:
	// Sets default values for this actor's properties
	ASimpleNavigationAffector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual uint8 GetFlowfieldNavigationCost_Implementation() override;

};
