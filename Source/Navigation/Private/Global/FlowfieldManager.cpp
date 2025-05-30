// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/FlowfieldManager.h"

#include "Flowfield/Flowfield.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AFlowfieldManager::AFlowfieldManager()
{
}

void AFlowfieldManager::BeginPlay()
{
	Super::BeginPlay();
	
}


AFlowfield* AFlowfieldManager::InitializeFlowfield()
{
	check(GetWorld());

	AActor* FlowfieldActor = UGameplayStatics::GetActorOfClass(GetWorld(), AFlowfield::StaticClass());
	checkf(IsValid(FlowfieldActor), TEXT("Flowfield was not found on the map!"));

	Flowfield = Cast<AFlowfield>(FlowfieldActor);
	Flowfield->Initialize();

	return Flowfield;
}

AFlowfield* AFlowfieldManager::GetFlowfield()
{
	if (IsValid(Flowfield))
	{
		return Flowfield;
	}

	InitializeFlowfield();
	return Flowfield;
}

