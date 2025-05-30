// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "RVOProcessor.generated.h"

class UCCSCollisionsSubsystem;
struct FClusterFragment;
struct FCollisionFragment;
class UCCSEntitiesManagerSubsystem;
class UCCSEntitiesHashGrid;
class UCrowdStatisticsSubsystem;
/**
 * 
 */
UCLASS()
class CLEVERCROWD_API URVOProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	struct FEntityProxyData
	{
		FTransform& Transform;
		const FVector& Location;
		const float Radius;
		FVector& Force;
		FCollisionFragment& CollisionFragment;
		FClusterFragment& ClusterFragment;
	};

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	UCCSEntitiesHashGrid* EntitiesHashGrid;
	UPROPERTY()
	UCrowdStatisticsSubsystem* CrowdStatisticsSubsystem;
	UPROPERTY()
	UCCSEntitiesManagerSubsystem* EntitiesManagerSubsystem;
	UPROPERTY()
	UCCSCollisionsSubsystem* CollisionsSubsystem;

	
public:
	
	URVOProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:

	void DoSimpleAvoidance(FEntityProxyData& EntityData, FEntityProxyData& OtherEntityData, float AvoidanceRadius, float AvoidanceStrength, float DeltaTime);
	void DoToTheSideAvoidance(FCollisionFragment& CollisionFragment, FVector& Force, FTransform& Transform, FMassExecutionContext& Context, float Duration, float DeltaTime);

	// ORCA
	void DoORCA(FEntityProxyData& EntityData, TArray<FEntityProxyData>& OtherEntityDatas, float AvoidanceRadius, float AvoidanceStrength, float DeltaTime);
	static FVector::FReal ComputeClosestPointOfApproach(const FVector RelPos, const FVector RelVel, const FVector::FReal TotalRadius, const FVector::FReal TimeHoriz);
};
