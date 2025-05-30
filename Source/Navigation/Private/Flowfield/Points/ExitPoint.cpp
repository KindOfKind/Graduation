// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/Points/ExitPoint.h"

#include "MassCommandBuffer.h"
#include "MassEntityManager.h"
#include "Entity/EntityNotifierSubsystem.h"


// Sets default values
AExitPoint::AExitPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AExitPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExitPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AExitPoint::InteractWithEntity(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity)
{
	if (!EntityManager.IsEntityActive(Entity))
	{
		return false;
	}
	GetWorld()->GetSubsystem<UEntityNotifierSubsystem>()->PreDestroyEntityDelegate.Execute(Entity);	// ToDo: cache the subsystem
	EntityManager.Defer().DestroyEntity(Entity);
	return true;
}

