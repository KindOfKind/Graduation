// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Common/CommonTypes.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Grids/UtilsGridTypes.h"


namespace GameEvaluator
{
}


template<typename T>
struct EVALUATOR_API TValueRange
{
	T Min;
	T Max;
	TValueRange() = default;
	TValueRange(T InMin, T InMax) : Min(InMin), Max(InMax) {};

	friend FArchive& operator <<(FArchive& Ar, TValueRange& Range)
	{
		Ar << Range.Min;
		Ar << Range.Max;
		return Ar;
	}
};

// Param that is calculated during tests to evaluate how well algorithms work
template<typename T>
struct EVALUATOR_API TEvaluatorMetricParam
{
private:
	T Value;

public:
	inline T& Get() { return Value; }

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetricParam& Param)
	{
		Ar << Param.Value;
		return Ar;
	}
};

template<typename T>
struct EVALUATOR_API TEvaluatorMetricClusterParam
{
private:
	TArray<T> ValuesInClusters;

public:
	TEvaluatorMetricClusterParam()
	{
		ValuesInClusters.AddDefaulted(FCrowdStatistics::MaxClusterType);
	}
	
	inline T& GetInClusterChecked(int32 ClusterType) { return ValuesInClusters[ClusterType]; }

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetricClusterParam& Param)
	{
		Ar << Param.ValuesInClusters;
		return Ar;
	}

	int32 GetClustersNum() { return ValuesInClusters.Num(); }
};

template<typename T>
struct EVALUATOR_API TEvaluatorMetricArealParam
{
private:
	TArray<T> ValuesInAreas;

public:
	TEvaluatorMetricArealParam()
	{
	}
	
	T GetValue(int32 MapAreaId)
	{
		if (!ValuesInAreas.IsValidIndex(MapAreaId))
		{
			return T();
		}
		return ValuesInAreas[MapAreaId];
	}
	
	void SetValue(int32 MapAreaId, const T& Value)
	{
		if (!ValuesInAreas.IsValidIndex(MapAreaId))
		{
			ValuesInAreas.AddDefaulted(MapAreaId - ValuesInAreas.Num() + 1);
		}
		ValuesInAreas[MapAreaId] = Value;
	}

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetricArealParam& Param)
	{
		Ar << Param.ValuesInAreas;
		return Ar;
	}

	int32 GetAreasNum() const { return ValuesInAreas.Num(); }
	bool IsValidAreaType(const int32 AreaTypeId) const { return (ValuesInAreas.Num() > AreaTypeId); }
};


// Param that is tweaked during tests to find the best value
template<typename T>
struct EVALUATOR_API TEvaluatorMetaParam
{
	T Value;              // Currently used value
	TValueRange<T> Range; // Range of possible values
	T Step;               // How much this value is changed per one step (test session / param randomization call)

	bool bIncrementOnly = false; // If true, Randomize() will always make Value bigger
	bool bIgnoreStep    = false; // If true, will ignore step and randomly select any value in range

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetaParam& Param)
	{
		Ar << Param.Value;
		Ar << Param.Range;
		Ar << Param.Step;
		return Ar;
	}

	void Randomize()
	{
		if (bIgnoreStep)
		{
			Value = FMath::RandRange(Range.Min, Range.Max);
		}
		else
		{
			int32 Mult = (FMath::RandBool() ? 1 : -1);
			if (bIncrementOnly) Mult = 1;
			Value += Step * Mult;
		}
		Value = FMath::Clamp(Value, Range.Min, Range.Max);
	};
};

template<typename T>
struct EVALUATOR_API TEvaluatorMetaArealParam
{
private:
	TArray<TEvaluatorMetaParam<T>> ParamsInAreas;
	TEvaluatorMetaParam<T> DefaultValue;

public:
	TEvaluatorMetaArealParam(TEvaluatorMetaParam<T> InDefaultValue)
	{
		DefaultValue = InDefaultValue;
	}

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetaArealParam& Param)
	{
		Ar << Param.ParamsInAreas;
		return Ar;
	}

	TArray<TEvaluatorMetaParam<T>>& Get() 
	{
		return ParamsInAreas;
	}
	const TArray<TEvaluatorMetaParam<T>>& Get() const
	{
		return ParamsInAreas;
	}
	
	T GetValue(int32 MapAreaId)
	{
		if (!ParamsInAreas.IsValidIndex(MapAreaId))
		{
			return T();
		}
		return ParamsInAreas[MapAreaId].Value;
	}

	void AddArea()
	{
		ParamsInAreas.Add(DefaultValue);
	}

	int32 GetAreasNum() { return ParamsInAreas.Num(); }

	void TryExpandToAreasNum(const int32 AreasNum)
	{
		const int32 AreasToAdd = AreasNum - ParamsInAreas.Num();
		for (int32 i = 0; i < AreasToAdd; i++)
		{
			AddArea();
		}
	}

	void Randomize()
	{
		for (TEvaluatorMetaParam<T>& MetaParam : ParamsInAreas)
		{
			MetaParam.Randomize();
		}
	}
};


struct EVALUATOR_API FEvaluatorMetaParamsContainer
{
	int32 TestsNum = 0;	// How many times this MetaParamsContainer has been used in tests (in future it will help to define how to change these params for the next test)
	TEvaluatorMetaParam<int32> DetourMaxAdditionalCost{35, {10, 70}, 0};	// Max additional costs from detour
	TEvaluatorMetaParam<int32> AgentsNum{4000, {20, 5000}, 0};
	TEvaluatorMetaParam<float> DecrementCollisionsCountRate{0.5f, {0.3f, 0.9f}, 0.0f};
	TEvaluatorMetaArealParam<float> AgentMovementSpeedAreal{{50.f, {40.f, 80.f}, 0.f}};
	TEvaluatorMetaArealParam<float> AvoidanceStrengthAreal{{0.5f, {0.f, 2.f}, 0.5f, false, false}};
	TEvaluatorMetaArealParam<float> AvoidanceRadiusAreal{{100.f, {0.f, 200.f}, 20.f}};
	TEvaluatorMetaArealParam<float> ToTheSideAvoidanceDurationAreal{{2.f, {0.f, 5.f}, 2.f}};
	TEvaluatorMetaParam<float> DefaultToTheSideAvoidanceDuration{2.f, {0.f, 5.f}, 2.f};
	
	TEvaluatorMetaParam<float> AgentMovementSpeedSpaciousCluster{50.f, {40.f, 80.f}, 0.f};
	TEvaluatorMetaParam<float> AgentMovementSpeedDenseCluster{50.f, {40.f, 80.f}, 0.f};
	TEvaluatorMetaParam<float> AvoidanceStrengthSpaciousCluster{0.5f, {0.f, 2.f}, 0.5f, false, false};
	TEvaluatorMetaParam<float> AvoidanceStrengthDenseCluster{0.f, {0.f, 5.f}, 0.f, false, true};
	TEvaluatorMetaParam<float> AvoidanceRadiusSpaciousCluster{100.f, {0.f, 200.f}, 20.f};
	TEvaluatorMetaParam<float> AvoidanceRadiusDenseCluster{120.f, {0.f, 200.f}, 20.f};

	TEvaluatorMetaParam<float> UserParam01{1.f, {1.f, 1.f}, 0.f};	// User params one may use to avoid incompatibility with old save files
	TEvaluatorMetaParam<float> UserParam02{1.f, {1.f, 1.f}, 0.f};
	TEvaluatorMetaParam<float> UserParam03{1.f, {1.f, 1.f}, 0.f};
	TEvaluatorMetaParam<float> UserParam04{1.f, {1.f, 1.f}, 0.f};
	TEvaluatorMetaParam<float> UserParam05{1.f, {1.f, 1.f}, 0.f};
	TEvaluatorMetaParam<float> UserParam06{1.f, {1.f, 1.f}, 0.f};

	friend FArchive& operator <<(FArchive& Ar, FEvaluatorMetaParamsContainer& Container)
	{
		// ToDo: make an array of references to these variables and iterate through them, because I know it's terrible now...
		Ar << Container.TestsNum;
		Ar << Container.DetourMaxAdditionalCost;
		Ar << Container.AgentsNum;
		Ar << Container.DecrementCollisionsCountRate;
		Ar << Container.AgentMovementSpeedAreal;
		Ar << Container.AvoidanceStrengthAreal;
		Ar << Container.AvoidanceRadiusAreal;
		Ar << Container.ToTheSideAvoidanceDurationAreal;
		Ar << Container.DefaultToTheSideAvoidanceDuration;
		
		Ar << Container.AgentMovementSpeedSpaciousCluster;
		Ar << Container.AgentMovementSpeedDenseCluster;
		Ar << Container.AvoidanceStrengthSpaciousCluster;
		Ar << Container.AvoidanceStrengthDenseCluster;
		Ar << Container.AvoidanceRadiusSpaciousCluster;
		Ar << Container.AvoidanceRadiusDenseCluster;
		
		Ar << Container.UserParam01;
		Ar << Container.UserParam02;
		Ar << Container.UserParam03;
		Ar << Container.UserParam04;
		Ar << Container.UserParam05;
		Ar << Container.UserParam06;
		return Ar;
	}

	void PrepareArealParams(int32 MaxAreaTypeOnLevel)
	{
		AgentMovementSpeedAreal.TryExpandToAreasNum(MaxAreaTypeOnLevel + 1);
		AvoidanceStrengthAreal.TryExpandToAreasNum(MaxAreaTypeOnLevel + 1);
		AvoidanceRadiusAreal.TryExpandToAreasNum(MaxAreaTypeOnLevel + 1);
		ToTheSideAvoidanceDurationAreal.TryExpandToAreasNum(MaxAreaTypeOnLevel + 1);
	}

	void RandomizeAllParams()
	{
		DetourMaxAdditionalCost.Randomize();
		AgentsNum.Randomize();
		DecrementCollisionsCountRate.Randomize();
		AgentMovementSpeedAreal.Randomize();
		AvoidanceStrengthAreal.Randomize();
		AvoidanceRadiusAreal.Randomize();
		ToTheSideAvoidanceDurationAreal.Randomize();
		DefaultToTheSideAvoidanceDuration.Randomize();
		
		AgentMovementSpeedSpaciousCluster.Randomize();
		AgentMovementSpeedDenseCluster.Randomize();
		AvoidanceStrengthSpaciousCluster.Randomize();
		AvoidanceStrengthDenseCluster.Randomize();
		AvoidanceRadiusSpaciousCluster.Randomize();
		AvoidanceRadiusDenseCluster.Randomize();
		
		UserParam01.Randomize();
		UserParam02.Randomize();
		UserParam03.Randomize();
		UserParam04.Randomize();
		UserParam05.Randomize();
		UserParam06.Randomize();
	}
};

struct EVALUATOR_API FEvaluatorMetricParamsContainer
{
	TEvaluatorMetricParam<float> TestDuration;
	TEvaluatorMetricParam<FAggregatedValueFloat> AggregatedTickTime;	// Used to calculate average delta time
	TEvaluatorMetricParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeed;	// Used to calculate average entities movement speed
	TEvaluatorMetricClusterParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeedInClusters;
	TEvaluatorMetricArealParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeedAreal;	// Used to calculate average entities movement speed
	TEvaluatorMetricClusterParam<int64> CollisionsCountInClusters;
	TEvaluatorMetricArealParam<int64> CollisionsCountAreal;
	TEvaluatorMetricParam<float> AverageEntityFinishedTime;

	TEvaluatorMetricParam<float> UserParam01;	// User params one may use to avoid incompatibility with old save files
	TEvaluatorMetricParam<float> UserParam02;
	TEvaluatorMetricParam<float> UserParam03;
	TEvaluatorMetricParam<float> UserParam04;
	TEvaluatorMetricParam<float> UserParam05;
	TEvaluatorMetricParam<float> UserParam06;

	friend FArchive& operator <<(FArchive& Ar, FEvaluatorMetricParamsContainer& Container)
	{
		Ar << Container.AggregatedTickTime;
		Ar << Container.TestDuration;
		Ar << Container.AggregatedEntitiesMovementSpeed;
		Ar << Container.AggregatedEntitiesMovementSpeedAreal;
		Ar << Container.AggregatedEntitiesMovementSpeedInClusters;
		Ar << Container.CollisionsCountAreal;
		Ar << Container.CollisionsCountInClusters;
		Ar << Container.AverageEntityFinishedTime;

		Ar << Container.UserParam01;
		Ar << Container.UserParam02;
		Ar << Container.UserParam03;
		Ar << Container.UserParam04;
		Ar << Container.UserParam05;
		Ar << Container.UserParam06;
		return Ar;
	}
};


struct EVALUATOR_API FCrowdAgentMetrics
{
	FVector Location               = FVector::ZeroVector;
	FGridCellPosition CellPosition = FGridCellPosition{};
	FVector MovementDirection      = FVector::ZeroVector;
	int32 WeakCollisions           = 0;
	int32 StrongCollisions         = 0;

	FCrowdAgentMetrics operator -(const FCrowdAgentMetrics& Other) const
	{
		FCrowdAgentMetrics Result;
		Result.Location          = Location - Other.Location;
		Result.CellPosition      = CellPosition - Other.CellPosition;
		Result.MovementDirection = MovementDirection - Other.MovementDirection;
		Result.StrongCollisions  = StrongCollisions - Other.StrongCollisions;
		Result.WeakCollisions    = WeakCollisions - Other.WeakCollisions;
		return Result;
	}

	friend FArchive& operator <<(FArchive& Ar, FCrowdAgentMetrics& Metrics)
	{
		Ar << Metrics.Location;
		Ar << Metrics.CellPosition;
		Ar << Metrics.MovementDirection;
		Ar << Metrics.StrongCollisions;
		Ar << Metrics.WeakCollisions;
		return Ar;
	}
};

// Can be used for normalization or "value accumulation" (summing across multiple snapshots)
struct EVALUATOR_API FCrowdAgentMetricsMag
{
	float Location          = 0.f;
	float PathWalked        = 0.f;
	float MovementDirection = 0.f;
	float WeakCollisions    = 0.f;
	float StrongCollisions  = 0.f;

	static constexpr int32 FloatMetricsNum = 5;

	float GetFloatMetric(const int32 MetricIdx) const
	{
		switch (MetricIdx)
		{
		case 0:
			return Location;
		case 1:
			return PathWalked;
		case 3:
			return MovementDirection;
		case 4:
			return WeakCollisions;
		case 5:
			return StrongCollisions;
		default:
			return 0.f;
		}
	}
	
	void ForEachFloatMetric(const TFunction<void(float)>& Callback) const
	{
		Callback(Location);
		Callback(PathWalked);
		Callback(MovementDirection);
		Callback(WeakCollisions);
		Callback(StrongCollisions);
	}
};


struct EVALUATOR_API FCrowdGroup
{
	FCrowdAgentMetricsMag AverageAgentMetrics;
	TArray<TArray<FGridBounds>> BoundsSnapshots;	// BoundsSnapshots[i][j], where i - Snapshot index, j - bound index (each bound contains a group of nearby units)
};


struct EVALUATOR_API FCrowdAgentMetricsSnapshot
{
	FCrowdAgentMetrics Metrics;
	int32 CrowdGroupIndex = 0;

	static constexpr float MaxDeltaLocation = 130.;	// Max delta per sec. 50 cm/s is a common agent movement speed.
	static constexpr float MaxRotation = 35.f;
	static constexpr int32 MaxDeltaWeakCollisions = 120;	// An optimal value is defined empirically
	static constexpr int32 MaxDeltaStrongCollisions = 10;	// An optimal value is defined empirically

	// Calculates metrics delta and normalizes metrics in some predefined optimal ranges.
	static FCrowdAgentMetricsMag GetSnapshotsDeltaNormalized(
		const FCrowdAgentMetricsSnapshot& StartSnapshot, const FCrowdAgentMetricsSnapshot& EndSnapshot,
		const FCrowdAgentMetricsMag& AccumulatedMetricValues, const float DeltaTime)
	{
		FCrowdAgentMetricsSnapshot DeltaSnapshot;
		DeltaSnapshot.Metrics = EndSnapshot.Metrics - StartSnapshot.Metrics;

		FCrowdAgentMetricsMag NormSnapshot;
		NormSnapshot.Location          = FMath::Clamp(DeltaSnapshot.Metrics.Location.Size() / (MaxDeltaLocation * DeltaTime), 0.0, 1.0);
		NormSnapshot.PathWalked        = FMath::Clamp(AccumulatedMetricValues.PathWalked / (MaxDeltaLocation * DeltaTime), 0.0, 1.0);
		NormSnapshot.MovementDirection = FMath::Clamp(AccumulatedMetricValues.MovementDirection / (MaxRotation * DeltaTime), 0.0, 1.0);
		NormSnapshot.WeakCollisions    = FMath::Clamp(AccumulatedMetricValues.WeakCollisions / (MaxDeltaWeakCollisions * DeltaTime), 0.0, 1.0);
		NormSnapshot.StrongCollisions  = FMath::Clamp(AccumulatedMetricValues.StrongCollisions / (MaxDeltaStrongCollisions * DeltaTime), 0.0, 1.0);

		return NormSnapshot;
	}

	static FCrowdAgentMetricsSnapshot GetSnapshotsDelta(const FCrowdAgentMetricsSnapshot& StartSnapshot, const FCrowdAgentMetricsSnapshot& EndSnapshot)
	{
		FCrowdAgentMetricsSnapshot DeltaSnapshot;
		DeltaSnapshot.Metrics = EndSnapshot.Metrics - StartSnapshot.Metrics;
		return DeltaSnapshot;
	}

	friend FArchive& operator <<(FArchive& Ar, FCrowdAgentMetricsSnapshot& Snapshot)
	{
		Ar << Snapshot.Metrics;
		Ar << Snapshot.CrowdGroupIndex;
		return Ar;
	}
};

struct EVALUATOR_API FCrowdAgentMetricsSnapshotsContainer
{
	TArray<FCrowdAgentMetricsSnapshot> Snapshots;

	friend FArchive& operator <<(FArchive& Ar, FCrowdAgentMetricsSnapshotsContainer& Container)
	{
		Ar << Container.Snapshots;
		return Ar;
	}
};

struct EVALUATOR_API FCrowdAgentsEvaluationResult
{
public:
	// Each CrowdGroup describes different average Crowd Agent metrics. Each agent is then assigned to a group it matches the most.
	// If no similar groups are found (low metrics correlation), a new one is created.
	// ToDo: rename CrowdGroup to AgentType, because crowd groups now is a different concept - crowd groups are collections of nearby agents 
	TArray<FCrowdGroup> CrowdGroups;
	
private:
	TMap<FMassEntityHandle, FCrowdAgentMetricsSnapshotsContainer> AgentsMetrics;
	TArray<FMassEntityHandle> SavableEntities;
	TArray<FCrowdAgentMetricsSnapshotsContainer> SavableMetrics;

public:
	friend FArchive& operator <<(FArchive& Ar, FCrowdAgentsEvaluationResult& Evaluation)
	{
		// AgentsMetrics TMap serialization
		// Currently commented because of FMassEntityHandle serialization issue
		
		// if (Ar.IsSaving())
		// {
		// 	const int32 AgentsNum = Evaluation.AgentsMetrics.Num();
		// 	Evaluation.SavableEntities.Reserve(AgentsNum);
		// 	Evaluation.SavableMetrics.Reserve(AgentsNum);
		// 	for (const auto& [EntityHandle, Snapshots] : Evaluation.AgentsMetrics)
		// 	{
		// 		Evaluation.SavableEntities.Add(EntityHandle);
		// 		Evaluation.SavableMetrics.Add(Snapshots);
		// 	}
		// 	Ar << Evaluation.SavableEntities;
		// 	Ar << Evaluation.SavableMetrics;
		// }
		// else if (Ar.IsLoading())
		// {
		// 	Ar << Evaluation.SavableEntities;
		// 	Ar << Evaluation.SavableMetrics;
		// 	for (int32 i = 0; i < Evaluation.SavableEntities.Num(); i++)
		// 	{
		// 		Evaluation.AgentsMetrics.Add(Evaluation.SavableEntities[i], Evaluation.SavableMetrics[i]);
		// 	}
		// }
		
		return Ar;
	}

	void Clear()
	{
		*this = FCrowdAgentsEvaluationResult();
	}

	void AddAgentMetricsSnapshot(const FMassEntityHandle& Entity, FCrowdAgentMetricsSnapshot& Snapshot)
	{
		FCrowdAgentMetricsSnapshotsContainer& SnapshotsContainer = AgentsMetrics.FindOrAdd(Entity);
		if (!SnapshotsContainer.Snapshots.IsEmpty())
		{
			Snapshot.CrowdGroupIndex = SnapshotsContainer.Snapshots.Last().CrowdGroupIndex;
		}
		SnapshotsContainer.Snapshots.Add(Snapshot);
	}

	bool IsValidSnapshotIndex(const FMassEntityHandle& Entity, const int32 Index)
	{
		if (FCrowdAgentMetricsSnapshotsContainer* SnapshotsContainer = AgentsMetrics.Find(Entity))
		{
			return SnapshotsContainer->Snapshots.IsValidIndex(Index);
		}
		return false;
	}

	// @note: Considers that each snapshot is made with a fixed SnapshotsRate
	int32 GetSnapshotIndexAtTimeSafe(const float Time, const float SnapshotsRate) const
	{
		int32 SnapshotIndex = (Time / SnapshotsRate) - 1;
		SnapshotIndex       = FMath::Max(SnapshotIndex, 0);
		return SnapshotIndex;
	}

	FCrowdAgentMetricsSnapshot& GetSnapshot(const FMassEntityHandle& Entity, const int32 SnapshotIndex)
	{
		FCrowdAgentMetricsSnapshotsContainer* SnapshotsContainer = AgentsMetrics.Find(Entity);
		if (SnapshotsContainer)
		{
			return SnapshotsContainer->Snapshots[SnapshotIndex];
		}
		else
		{
			return SnapshotsContainer->Snapshots.Last();
		}
	}
	
	FCrowdAgentMetricsSnapshot& GetLastSnapshot(const FMassEntityHandle& Entity)	// Unsafe
	{
		FCrowdAgentMetricsSnapshotsContainer* SnapshotsContainer = AgentsMetrics.Find(Entity);
		return SnapshotsContainer->Snapshots.Last();
	}

	// @note: Considers that each snapshot is made with a fixed SnapshotsRate
	FCrowdAgentMetricsSnapshot* GetAgentMetricsSnapshotAtTime(const FMassEntityHandle& Entity, const float Time, const float SnapshotsRate)
	{
		FCrowdAgentMetricsSnapshotsContainer* SnapshotsContainer = AgentsMetrics.Find(Entity);
		if (!SnapshotsContainer) return nullptr;
		const int32 SnapshotIndex = GetSnapshotIndexAtTimeSafe(Time, SnapshotsRate);
		if (SnapshotIndex < 0) return nullptr;
		return &SnapshotsContainer->Snapshots[SnapshotIndex];
	}
};
