// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CCSMovementProcessor.generated.h"

class UCCSEntitiesManagerSubsystem;
/**
 * 
 */
UCLASS()
class CLEVERCROWD_API UCCSMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;
	UCCSEntitiesManagerSubsystem* EntitiesManagerSubsystem;
	
public:
	
	UCCSMovementProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

};
