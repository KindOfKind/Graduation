// Fill out your copyright notice in the Description page of Project Settings.


#include "Flowfield/NavigationAffector/SimpleNavigationAffector.h"

#include "Global/NavigationGlobals.h"


// Sets default values
ASimpleNavigationAffector::ASimpleNavigationAffector()
{
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	RootComponent = SceneComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);
	StaticMeshComponent->SetCollisionProfileName(UE::NavigationGlobals::NavigationAffectorProfileName);

	NavigationCost = UE::NavigationGlobals::MaxCost;
}

// Called when the game starts or when spawned
void ASimpleNavigationAffector::BeginPlay()
{
	Super::BeginPlay();
	
}

uint8 ASimpleNavigationAffector::GetFlowfieldNavigationCost_Implementation()
{
	return NavigationCost;
}

