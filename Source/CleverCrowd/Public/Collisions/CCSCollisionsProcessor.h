// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "CCSCollisionsProcessor.generated.h"

struct FCollisionFragment;
class UCrowdStatisticsSubsystem;
struct FTransformFragment;
class UCCSEntitiesHashGrid;
class UCCSCollisionsSubsystem;

UCLASS()
class CLEVERCROWD_API UCCSCollisionsProcessor : public UMassProcessor
{
	GENERATED_BODY()

private:

	struct FEntityProxyData
	{
		FTransform& Transform;
		const FVector& Location;
		const float Radius;
		FCollisionFragment& CollisionFragment;
	};

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	UCCSEntitiesHashGrid* EntitiesHashGrid;
	UPROPERTY()
	UCCSCollisionsSubsystem* CollisionsSubsystem;
	UPROPERTY()
	UCrowdStatisticsSubsystem* CrowdStatisticsSubsystem;
	
public:
	
	UCCSCollisionsProcessor();

protected:
	
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:

	void ResolveTwoAgentsCollisions(bool& bOutCollisionOccured, FEntityProxyData& EntityData, FEntityProxyData& OtherEntityData);
	void ResolveAgentCollisionsWithObstacles(FEntityProxyData& EntityData);
	void TryIncreaseCollisionsCount(bool bCollisionOccured, const FVector& Location) const;

	// DEBUG ------

	void DrawDebugVerticalLineInGameThread(UWorld* World, const FVector& Location, const float LifeTime = 0.f);

};
