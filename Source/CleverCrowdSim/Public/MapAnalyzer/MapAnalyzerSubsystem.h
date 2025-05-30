// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapAreaAnalyzer.h"
#include "MapClusterizationTypes.h"
#include "Grids/UtilsGridTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapAnalyzerSubsystem.generated.h"


class ACrowdEvaluationHashGrid;
class UCCSCollisionsSubsystem;
class UCrowdNavigationSubsystem;
class UCCSObstaclesHashGrid;
class UClustersHashGrid;


namespace UE::Geometry
{
	struct FClusterKMeans;
}


UCLASS()
class CLEVERCROWDSIM_API UMapAnalyzerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class ACCSGameMode;

private:
	UPROPERTY()
	FClusterizationRules ClusterizationRules;
	UPROPERTY()
	TObjectPtr<UClustersHashGrid> ClustersHashGrid;
	UPROPERTY()
	TObjectPtr<UCrowdNavigationSubsystem> CrowdNavigationSubsystem;
	UPROPERTY()
	TObjectPtr<UCCSCollisionsSubsystem> CollisionsSubsystem;
	UPROPERTY()
	TObjectPtr<ACrowdEvaluationHashGrid> CrowdEvaluationGrid;

	TMultiMap<int32, FMapAreaData> MapAreasDataConfig;	// Index - MapArea type id. Config means these are "reference" values that have been evaluated during tests and now are used for classification.
	int32 MaxMapAreaTypeId = 0;
	
protected:
	virtual void PostInitialize() override;

public:
	void SetMapClusterizationRules(const FClusterizationRules& InClusterizationRules);
	// Scans objects on a map, divides the map into clusters and caches cluster types in clusters hash grid.  
	void DefineClustersOnMap();

	UClustersHashGrid* GetClustersHashGrid();
	UCCSObstaclesHashGrid* GetObstaclesHashGrid();

	void DefineMapAreasOnMap();
	void DefineMapAreaInBounds(const FGridBounds& Bounds);
	void EvaluateMapAreaDataInBounds(FMapAreaData& OutAreaData, const FGridBounds& Bounds);

private:
	void InitializeOuter();
	
	void GroupPointsIntoClusters(TArray<TArray<int32>>& OutClusteredPoints, UE::Geometry::FClusterKMeans& Clustering,
	                             const FClusterizationRules& InClusterizationRules, const TArray<FVector>& PointsLocations);
};
