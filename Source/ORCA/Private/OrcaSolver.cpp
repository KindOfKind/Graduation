// Fill out your copyright notice in the Description page of Project Settings.


#include "OrcaSolver.h"
#include "ORCA/ThirdParty/RVO2/src/RVO.h"

void UOrcaSolver::BeginDestroy()
{
	delete simulator;
	
	UObject::BeginDestroy();
}

void UOrcaSolver::Initialize(const FOrcaDefaultAgentParams& AgentDefaults)
{
	simulator = new RVO::RVOSimulator();

	/* Specify the global time step of the simulation. */
	simulator->setTimeStep(0.25F);

	/* Specify the default parameters for agents that are subsequently added. */
	simulator->setAgentDefaults(AgentDefaults.NeighborDist, 10U, 10.0F, 10.0F, AgentDefaults.AgentsRadius, AgentDefaults.MaxSpeed);
}

int32 UOrcaSolver::AddAgent(const FVector& Location)
{
	return simulator->addAgent(RVO::Vector2{static_cast<float>(Location.X), static_cast<float>(Location.Y)});
}

void UOrcaSolver::DeleteAgent(const int32 AgentIndex, const FVector& Location)
{
	// Deleting agent is an issue to solve. Currently, we simply teleport agents far away :_)
	constexpr float BigFloat = MAX_FLT - 10000.f;
	simulator->setAgentPosition(AgentIndex, RVO::Vector2{BigFloat, BigFloat});
}

void UOrcaSolver::SetAgentLocation(const int32 AgentIndex, const FVector& Location)
{
	simulator->setAgentPosition(AgentIndex, RVO::Vector2{static_cast<float>(Location.X), static_cast<float>(Location.Y)});
}

FVector UOrcaSolver::GetAgentLocation(const int32 AgentIndex) const
{
	RVO::Vector2 Result = simulator->getAgentPosition(AgentIndex);
	return FVector{Result.x(), Result.y(), 0.0f};
}

void UOrcaSolver::DoStep()
{
	simulator->doStep();
}

void UOrcaSolver::SetTimeStep(const float TimeStep)
{
	simulator->setTimeStep(TimeStep);
}

void UOrcaSolver::SetPreferredVelocity(const int32 AgentIndex, const FVector& Direction)
{
	simulator->setAgentPrefVelocity(AgentIndex, RVO::Vector2{static_cast<float>(Direction.X), static_cast<float>(Direction.Y)});
}
