// Fill out your copyright notice in the Description page of Project Settings.


#include "Global/CrowdNavigationSubsystem.h"

#include "Global/FlowfieldManager.h"

UCrowdNavigationSubsystem::UCrowdNavigationSubsystem()
{
	bNavigationInitialized = false;
}

void UCrowdNavigationSubsystem::InitializeNavigation()
{
	InitializeFlowfieldManager();
	InitializeFlowfield();
}

void UCrowdNavigationSubsystem::InitializeFlowfieldManager()
{
	check(GetWorld());

	FlowfieldManager = GetWorld()->SpawnActor<AFlowfieldManager>(FActorSpawnParameters());
	check(IsValid(FlowfieldManager));
}

void UCrowdNavigationSubsystem::InitializeFlowfield()
{
	FlowfieldManager->InitializeFlowfield();
}

AFlowfieldManager* UCrowdNavigationSubsystem::GetFlowfieldManager()
{
	if (IsValid(FlowfieldManager))
	{
		return FlowfieldManager;
	}

	InitializeFlowfieldManager();

	return FlowfieldManager;
}

AFlowfield* UCrowdNavigationSubsystem::GetFlowfield()
{
	return GetFlowfieldManager()->GetFlowfield();
}
