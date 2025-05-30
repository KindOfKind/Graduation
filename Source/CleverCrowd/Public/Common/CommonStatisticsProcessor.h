// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CommonStatisticsProcessor.generated.h"


class UCrowdStatisticsSubsystem;

UCLASS()
class CLEVERCROWD_API UCommonStatisticsProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	TObjectPtr<UCrowdStatisticsSubsystem> CrowdStatistics;
	
public:
	
	UCommonStatisticsProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
};
