// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CrowdMetricsProcessor.generated.h"

struct FNavigationFragment;
class UCrowdNavigationSubsystem;
struct FCrowdAgentMetricsMag;
class UCCSEntitiesHashGrid;
struct FCollisionFragment;
struct FCrowdAgentMetrics;
struct FClusterFragment;
struct FTransformFragment;
class UGameEvaluatorSubsystem;
/**
 * Processor that gathers data about each individual crowd agent
 */
UCLASS()
class EVALUATOR_API UCrowdMetricsProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	TObjectPtr<UGameEvaluatorSubsystem> GameEvaluator;
	UPROPERTY()
	UCCSEntitiesHashGrid* EntitiesHashGrid;
	UPROPERTY()
	UCrowdNavigationSubsystem* CrowdNavigationSubsystem;

	// CONFIG START ------

	static constexpr float MetricsSnapshotRate = 1.f;
	static constexpr int32 CrowdRegroupRate    = 10; // Each 10 snapshots
	static constexpr bool bDebugCrowdGroup     = true;
	
	// CONFIG END

	float NextSnapshotTime       = MetricsSnapshotRate;
	int32 SnapshotsBeforeRegroup = CrowdRegroupRate;

	float CurrentTime;

	struct FEntityData
	{
		FMassEntityHandle Entity;
		FTransform& Transform;
		FClusterFragment& ClusterFragment;
		FCollisionFragment& CollisionFragment;
		FNavigationFragment& NavigationFragment;
	};

	float TestMaxWeakCollisionsCached = 0;
	float TestMaxStrongCollisionsCached = 0;
	
public:
	
	UCrowdMetricsProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	virtual void MakeSnapshot(FMassEntityManager& EntityManager, const FEntityData& EntityData);
	virtual int32 ReassignCrowdGroupToEntity(FMassEntityManager& EntityManager, const FEntityData& EntityData);
	virtual float CalculateAgentsSimilarityCoef(const FCrowdAgentMetricsMag& Metrics, const FCrowdAgentMetricsMag& OtherMetrics);
	virtual void InitDefaultCrowdGroup();

	void DebugDrawCrowdGroupOfEntity(FMassEntityManager& EntityManager, const FEntityData& EntityData);
};
