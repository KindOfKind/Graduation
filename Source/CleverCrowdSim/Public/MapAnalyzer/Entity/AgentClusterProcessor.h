// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "AgentClusterProcessor.generated.h"


class UMapAnalyzerSubsystem;
class UCCSObstaclesHashGrid;

UCLASS()
class CLEVERCROWDSIM_API UAgentClusterProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;
	UPROPERTY()
	UMapAnalyzerSubsystem* MapAnalyzerSubsystem;
	
public:
	
	UAgentClusterProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
