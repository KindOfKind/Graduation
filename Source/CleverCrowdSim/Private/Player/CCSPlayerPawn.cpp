// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CCSPlayerPawn.h"

#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Global/Config/CCSGlobals.h"


ACCSPlayerPawn::ACCSPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComponent;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");
	FloatingPawnMovement->UpdatedComponent = SceneComponent;

	HeightAboveFloor = 1000.f;
}

void ACCSPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACCSPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//AdjustFloorHeight();
}

void ACCSPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACCSPlayerPawn::AdjustFloorHeight()
{
	FHitResult HitResult;
	ECollisionChannel CollisionChannel = CCSGlobals::WalkableSurfaceChannel;
	const FVector PawnLocation         = GetActorLocation();
	const FVector TopTraceLocation     = PawnLocation + FVector{0.f, 0.f, 2000.f};
	const FVector BottomTraceLocation  = PawnLocation - HeightAboveFloor - FVector{0.f, 0.f, 2000.f};
	
	GetWorld()->LineTraceSingleByChannel(HitResult, TopTraceLocation, BottomTraceLocation, CollisionChannel, FCollisionQueryParams::DefaultQueryParam);

	if (HitResult.bBlockingHit)
	{
		FVector NewPawnLocation = GetActorLocation();
		NewPawnLocation.Z       = HitResult.Location.Z + HeightAboveFloor;;
		SetActorLocation(NewPawnLocation);
	}
}
