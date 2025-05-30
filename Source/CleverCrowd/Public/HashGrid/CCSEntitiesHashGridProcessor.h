// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CCSEntitiesHashGridProcessor.generated.h"

class UCCSEntitiesManagerSubsystem;
/**
 * 
 */
UCLASS()
class CLEVERCROWD_API UCCSEntitiesHashGridProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;
	
	UPROPERTY()
	UCCSEntitiesManagerSubsystem* EntitiesManager;
	
public:
	
	UCCSEntitiesHashGridProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
