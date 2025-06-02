// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Common/CommonTypes.h"
#include "Global/CrowdStatisticsSubsystem.h"
#include "Grids/UtilsGridTypes.h"

#define GET_VARIABLE_NAME(variable) #variable

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
	FString Name = "None";

public:
	TEvaluatorMetricParam() = delete;
	TEvaluatorMetricParam(const FString& InName) : Name(InName)
	{
		Value = T();
	};
	TEvaluatorMetricParam(const T& InValue, const FString& InName) : Value(InValue), Name(InName) {};
	
	inline T& Get() { return Value; }
	inline const T& Get() const { return Value; }

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetricParam& Param)
	{
		Ar << Param.Value;
		Ar << Param.Name;
		return Ar;
	}

	void WriteNameIntoString(FString& OutString, const FString& Delimiter = ",") const
	{
		OutString += Name + Delimiter;
	}
};

template<typename T>
struct EVALUATOR_API TEvaluatorMetricClusterParam
{
private:
	TArray<T> ValuesInClusters;
	FString Name = "None";

public:
	TEvaluatorMetricClusterParam() = delete;
	TEvaluatorMetricClusterParam(const FString& InName) : Name(InName)
	{
		ValuesInClusters.AddDefaulted(FCrowdStatistics::MaxClusterType);
	}
	
	inline T& GetInClusterChecked(int32 ClusterType) { return ValuesInClusters[ClusterType]; }
	inline const T& GetInClusterChecked(int32 ClusterType) const { return ValuesInClusters[ClusterType]; }

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetricClusterParam& Param)
	{
		Ar << Param.ValuesInClusters;
		Ar << Param.Name;
		return Ar;
	}

	const FString& GetName() const { return Name; }
	
	void WriteNameIntoString(FString& OutString, const FString& Delimiter = ",") const
	{
		for (int i = 0; i < ValuesInClusters.Num(); i++)
		{
			OutString += Name + "Clust" + FString::FromInt(i) + Delimiter;
		}
	}

	int32 GetClustersNum() const { return ValuesInClusters.Num(); }
};

template<typename T>
struct EVALUATOR_API TEvaluatorMetricArealParam
{
private:
	TArray<T> ValuesInAreas;
	FString Name = "None";

public:
	TEvaluatorMetricArealParam() = delete;
	TEvaluatorMetricArealParam(const FString& InName) : Name(InName) {};

	const T& GetValueChecked(int32 MapAreaId) const { return ValuesInAreas[MapAreaId]; };
	T& GetValueMutable(int32 MapAreaId) { return ValuesInAreas[MapAreaId]; };
	
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
		Ar << Param.Name;
		return Ar;
	}

	const FString& GetName() const { return Name; }
	
	void WriteNameIntoString(FString& OutString, const FString& Delimiter = ",") const
	{
		for (int i = 0; i < ValuesInAreas.Num(); i++)
		{
			OutString += Name + "Area" + FString::FromInt(i) + Delimiter;
		}
	}

	int32 GetAreasNum() const { return ValuesInAreas.Num(); }
	bool IsValidAreaType(const int32 AreaTypeId) const { return (ValuesInAreas.Num() > AreaTypeId); }
};


// Param that is tweaked during tests to find the best value
template<typename T>
struct EVALUATOR_API TEvaluatorMetaParam
{
	FString Name = "None";
	
	T Value;              // Currently used value
	TValueRange<T> Range; // Range of possible values
	T Step;               // How much this value is changed per one step (test session / param randomization call)

	bool bIncrementOnly = false; // If true, Randomize() will always make Value bigger
	bool bIgnoreStep    = false; // If true, will ignore step and randomly select any value in range

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetaParam& Param)
	{
		Ar << Param.Name;
		Ar << Param.Value;
		Ar << Param.Range;
		Ar << Param.Step;
		Ar << Param.bIncrementOnly;
		Ar << Param.bIgnoreStep;
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
	TEvaluatorMetaArealParam(const TEvaluatorMetaParam<T>& InDefaultValue)
	{
		DefaultValue = InDefaultValue;
	}

	friend FArchive& operator <<(FArchive& Ar, TEvaluatorMetaArealParam& Param)
	{
		Ar << Param.ParamsInAreas;
		return Ar;
	}

	void WriteNameIntoString(FString& OutString, const FString& Delimiter = ",") const
	{
		for (int i = 0; i < ParamsInAreas.Num(); i++)
		{
			OutString += ParamsInAreas[i].Name + "Area" + FString::FromInt(i) + Delimiter;
		}
	}

	TArray<TEvaluatorMetaParam<T>>& Get() 
	{
		return ParamsInAreas;
	}
	const TArray<TEvaluatorMetaParam<T>>& Get() const
	{
		return ParamsInAreas;
	}

	const T& GetValueChecked(int32 MapAreaId) const { return ParamsInAreas[MapAreaId].Value; };
	T GetValue(int32 MapAreaId) const
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

	const int32 GetAreasNum() const { return ParamsInAreas.Num(); }

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
	TEvaluatorMetaParam<int32> DetourMaxAdditionalCost{"MaxDetourCost", 15, {5, 60}, 15, false, false};	// Max additional costs from detour
	TEvaluatorMetaParam<int32> AgentsNum{"AgentsNum", 4000, {20, 5000}, 0};
	TEvaluatorMetaParam<float> DecrementCollisionsCountRate{"DecColCountRate", 0.5f, {0.3f, 0.9f}, 0.0f};
	TEvaluatorMetaArealParam<float> AgentMovementSpeedAreal{{"Speed", 50.f, {40.f, 80.f}, 0.f}};
	TEvaluatorMetaArealParam<float> AvoidanceStrengthAreal{{"AvoStr", 1.f, {0.f, 2.f}, 0.0f, false, false}};
	TEvaluatorMetaArealParam<float> AvoidanceRadiusAreal{{"AvoRad", 120.f, {0.f, 200.f}, 30.f, false, false}};
	TEvaluatorMetaArealParam<float> ToTheSideAvoidanceDurationAreal{{"SideAvoDur", 4.f, {0.f, 5.f}, 0.f, false, false}};
	TEvaluatorMetaParam<float> DefaultToTheSideAvoidanceDuration{"SideAvoDur", 4.f, {0.f, 5.f}, 0.f, false, false};
	TEvaluatorMetaParam<int32> AvoidanceType{"AvoidanceType", 1, {0, 2}, 1, false, true};	// Avoidance algorithms switcher. Currently, for all areas at once.
	
	TEvaluatorMetaParam<float> AgentMovementSpeedSpaciousCluster{"SpeedClustSpac", 50.f, {40.f, 80.f}, 0.f};
	TEvaluatorMetaParam<float> AgentMovementSpeedDenseCluster{"SpeedClustDense", 50.f, {40.f, 80.f}, 0.f};
	TEvaluatorMetaParam<float> AvoidanceStrengthSpaciousCluster{"AvoStrClustSpac", 1.f, {0.f, 2.f}, 0.f, false, false};
	TEvaluatorMetaParam<float> AvoidanceStrengthDenseCluster{"AvoStrClustDense", 0.8f, {0.f, 5.f}, 0.f, false, false};
	TEvaluatorMetaParam<float> AvoidanceRadiusSpaciousCluster{"AvoRadClustSpac", 120.f, {0.f, 200.f}, 0.f, false, false};
	TEvaluatorMetaParam<float> AvoidanceRadiusDenseCluster{"AvoRadClustDense", 120.f, {0.f, 200.f}, 0.f, false, false};

	TEvaluatorMetaParam<float> UserParam01{"UserParam01", 1.f, {1.f, 1.f}, 0.f};	// User params one may use to avoid incompatibility with old save files
	TEvaluatorMetaParam<float> UserParam02{"UserParam02", 1.f, {1.f, 1.f}, 0.f};

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
		Ar << Container.AvoidanceType;
		
		Ar << Container.AgentMovementSpeedSpaciousCluster;
		Ar << Container.AgentMovementSpeedDenseCluster;
		Ar << Container.AvoidanceStrengthSpaciousCluster;
		Ar << Container.AvoidanceStrengthDenseCluster;
		Ar << Container.AvoidanceRadiusSpaciousCluster;
		Ar << Container.AvoidanceRadiusDenseCluster;
		
		Ar << Container.UserParam01;
		Ar << Container.UserParam02;
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
		AvoidanceType.Randomize();
		
		AgentMovementSpeedSpaciousCluster.Randomize();
		AgentMovementSpeedDenseCluster.Randomize();
		AvoidanceStrengthSpaciousCluster.Randomize();
		AvoidanceStrengthDenseCluster.Randomize();
		AvoidanceRadiusSpaciousCluster.Randomize();
		AvoidanceRadiusDenseCluster.Randomize();
		
		UserParam01.Randomize();
		UserParam02.Randomize();
	}

	void WriteParamsNamesIntoString(FString& OutString) const
	{
		OutString += DetourMaxAdditionalCost.Name + ",";
		OutString += AgentsNum.Name + ",";
		OutString += DecrementCollisionsCountRate.Name + ",";
		
		AgentMovementSpeedAreal.WriteNameIntoString(OutString);
		AvoidanceStrengthAreal.WriteNameIntoString(OutString);
		AvoidanceRadiusAreal.WriteNameIntoString(OutString);
		ToTheSideAvoidanceDurationAreal.WriteNameIntoString(OutString);
		OutString += DefaultToTheSideAvoidanceDuration.Name + ",";
		OutString += AvoidanceType.Name + ",";
		
		OutString += AgentMovementSpeedSpaciousCluster.Name + ",";
		OutString += AgentMovementSpeedDenseCluster.Name + ",";
		OutString += AvoidanceStrengthSpaciousCluster.Name + ",";
		OutString += AvoidanceStrengthDenseCluster.Name + ",";
		OutString += AvoidanceRadiusSpaciousCluster.Name + ",";
		OutString += AvoidanceRadiusDenseCluster.Name;
		OutString += "\n";
	}

	void WriteParamsValuesIntoString(FString& OutString) const
	{
		OutString += FString::FromInt(DetourMaxAdditionalCost.Value) + ",";
		OutString += FString::FromInt(AgentsNum.Value) + ",";
		OutString += FString::SanitizeFloat(DecrementCollisionsCountRate.Value) + ",";

		for (int32 i = 0; i < AgentMovementSpeedAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(AgentMovementSpeedAreal.GetValueChecked(i)) + ",";
		}
		for (int32 i = 0; i < AvoidanceStrengthAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(AvoidanceStrengthAreal.GetValueChecked(i)) + ",";
		}
		for (int32 i = 0; i < AvoidanceRadiusAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(AvoidanceRadiusAreal.GetValueChecked(i)) + ",";
		}
		for (int32 i = 0; i < ToTheSideAvoidanceDurationAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(ToTheSideAvoidanceDurationAreal.GetValueChecked(i)) + ",";
		}
		OutString += FString::SanitizeFloat(DefaultToTheSideAvoidanceDuration.Value) + ",";
		OutString += FString::FromInt(AvoidanceType.Value) + ",";
		
		OutString += FString::SanitizeFloat(AgentMovementSpeedSpaciousCluster.Value) + ",";
		OutString += FString::SanitizeFloat(AgentMovementSpeedDenseCluster.Value) + ",";
		OutString += FString::SanitizeFloat(AvoidanceStrengthSpaciousCluster.Value) + ",";
		OutString += FString::SanitizeFloat(AvoidanceStrengthDenseCluster.Value) + ",";
		OutString += FString::SanitizeFloat(AvoidanceRadiusSpaciousCluster.Value) + ",";
		OutString += FString::SanitizeFloat(AvoidanceRadiusDenseCluster.Value);
		OutString += "\n";
	}
};

struct EVALUATOR_API FEvaluatorMetricParamsContainer
{
	TEvaluatorMetricParam<float> TestDuration{"TestDuration"};
	TEvaluatorMetricParam<FAggregatedValueFloat> AggregatedTickTime{"AvgTickTime"};	// Used to calculate average delta time
	TEvaluatorMetricParam<FAggregatedValueFloat> AggregatedMassProcExecutionTime{"AvgMassProcTime"};	// Used to calculate average execution time of all mass processors during one tick
	TEvaluatorMetricParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeed{"AvgSpeed"};	// Used to calculate average entities movement speed
	TEvaluatorMetricClusterParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeedInClusters{"AvgSpeed"};
	TEvaluatorMetricArealParam<FAggregatedValueFloat> AggregatedEntitiesMovementSpeedAreal{"AvgSpeed"};	// Used to calculate average entities movement speed
	TEvaluatorMetricClusterParam<int64> CollisionsCountInClusters{"CollisCount"};
	TEvaluatorMetricArealParam<int64> CollisionsCountAreal{"CollisCount"};
	TEvaluatorMetricArealParam<float> AverageEntityTimeInAreas{"AvgTimeIn"};
	TEvaluatorMetricParam<float> AverageEntityFinishedTime{"AvgFinishTime"};

	TEvaluatorMetricParam<float> UserParam01{"UserParam01"};	// User params one may use to avoid incompatibility with old save files
	TEvaluatorMetricParam<float> UserParam02{"UserParam02"};

	friend FArchive& operator <<(FArchive& Ar, FEvaluatorMetricParamsContainer& Container)
	{
		Ar << Container.TestDuration;
		Ar << Container.AggregatedTickTime;
		Ar << Container.AggregatedTickTime;
		Ar << Container.AggregatedMassProcExecutionTime;
		Ar << Container.AggregatedEntitiesMovementSpeedInClusters;
		Ar << Container.AggregatedEntitiesMovementSpeedAreal;
		Ar << Container.CollisionsCountInClusters;
		Ar << Container.CollisionsCountAreal;
		Ar << Container.AverageEntityTimeInAreas;
		Ar << Container.AverageEntityFinishedTime;

		Ar << Container.UserParam01;
		Ar << Container.UserParam02;
		return Ar;
	}

	void WriteParamsNamesIntoString(FString& OutString) const
	{
		TestDuration.WriteNameIntoString(OutString);
		AggregatedTickTime.WriteNameIntoString(OutString);
		AggregatedMassProcExecutionTime.WriteNameIntoString(OutString);
		AggregatedEntitiesMovementSpeed.WriteNameIntoString(OutString);
		AggregatedEntitiesMovementSpeedInClusters.WriteNameIntoString(OutString);
		AggregatedEntitiesMovementSpeedAreal.WriteNameIntoString(OutString);
		CollisionsCountInClusters.WriteNameIntoString(OutString);
		CollisionsCountAreal.WriteNameIntoString(OutString);
		AverageEntityTimeInAreas.WriteNameIntoString(OutString);
		AverageEntityFinishedTime.WriteNameIntoString(OutString, "");
		OutString += "\n";
	}

	void WriteParamsValuesIntoString(FString& OutString) const
	{
		OutString += FString::SanitizeFloat(TestDuration.Get()) + ",";
		OutString += FString::SanitizeFloat(AggregatedTickTime.Get().GetMean()) + ",";
		OutString += FString::SanitizeFloat(AggregatedMassProcExecutionTime.Get().GetMean()) + ",";
		OutString += FString::SanitizeFloat(AggregatedEntitiesMovementSpeed.Get().GetMean()) + ",";
		
		for (int32 i = 0; i < AggregatedEntitiesMovementSpeedInClusters.GetClustersNum(); i++)
		{
			OutString += FString::SanitizeFloat(AggregatedEntitiesMovementSpeedInClusters.GetInClusterChecked(i).GetMean()) + ",";
		}
		for (int32 i = 0; i < AggregatedEntitiesMovementSpeedAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(AggregatedEntitiesMovementSpeedAreal.GetValueChecked(i).GetMean()) + ",";
		}
		
		for (int32 i = 0; i < CollisionsCountInClusters.GetClustersNum(); i++)
		{
			OutString += FString::SanitizeFloat(CollisionsCountInClusters.GetInClusterChecked(i)) + ",";
		}
		for (int32 i = 0; i < CollisionsCountAreal.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(CollisionsCountAreal.GetValueChecked(i)) + ",";
		}
		for (int32 i = 0; i < AverageEntityTimeInAreas.GetAreasNum(); i++)
		{
			OutString += FString::SanitizeFloat(AverageEntityTimeInAreas.GetValueChecked(i)) + ",";
		}

		OutString += FString::SanitizeFloat(AverageEntityFinishedTime.Get());

		OutString += "\n";
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
