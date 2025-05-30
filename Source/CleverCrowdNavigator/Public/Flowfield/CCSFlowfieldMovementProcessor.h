// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CCSFlowfieldMovementProcessor.generated.h"

class UCrowdNavigationSubsystem;

UCLASS()
class CLEVERCROWDNAVIGATOR_API UCCSFlowfieldMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;

	static constexpr bool bDetourAvailable = true; 

	UPROPERTY()
	UCrowdNavigationSubsystem* CrowdNavigationSubsystem;
	
public:
	
	UCCSFlowfieldMovementProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
