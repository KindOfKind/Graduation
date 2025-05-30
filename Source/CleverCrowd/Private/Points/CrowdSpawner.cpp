// Fill out your copyright notice in the Description page of Project Settings.


#include "Points/CrowdSpawner.h"


ACrowdSpawner::ACrowdSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACrowdSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ACrowdSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

