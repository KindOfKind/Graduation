// Fill out your copyright notice in the Description page of Project Settings.


#include "Collisions/Obstacles/CCSSimpleObstacle.h"

#include "Components/BoxComponent.h"


ACCSSimpleObstacle::ACCSSimpleObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComp;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComp->SetupAttachment(RootSceneComp);
}

void ACCSSimpleObstacle::BeginPlay()
{
	Super::BeginPlay();
}

