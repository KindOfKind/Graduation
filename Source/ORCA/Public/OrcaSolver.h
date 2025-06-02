// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OrcaSolver.generated.h"

namespace RVO
{
	class RVOSimulator;
}

USTRUCT()
struct ORCA_API FOrcaDefaultAgentParams
{
	GENERATED_BODY()

	float NeighborDist = 200.f;
	float AgentsRadius = 40.f;
	float MaxSpeed     = 50.f;
};

/**
 * 
 */
UCLASS()
class ORCA_API UOrcaSolver : public UObject
{
	GENERATED_BODY()

private:
	RVO::RVOSimulator* simulator;

public:
	virtual void BeginDestroy() override;
		
public:
	void Initialize(const FOrcaDefaultAgentParams& AgentDefaults);
	int32 AddAgent(const FVector& Location);	// Returns agent index in Orca Solver
	void DeleteAgent(const int32 AgentIndex, const FVector& Location);	// Returns agent index in Orca Solver
	void SetAgentLocation(const int32 AgentIndex, const FVector& Location);
	FVector GetAgentLocation(const int32 AgentIndex) const;
	void DoStep();
	void SetTimeStep(const float TimeStep);
	void SetPreferredVelocity(const int32 AgentIndex, const FVector& Direction);
};
