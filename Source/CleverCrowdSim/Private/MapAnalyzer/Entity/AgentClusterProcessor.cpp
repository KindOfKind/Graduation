// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAnalyzer/Entity/AgentClusterProcessor.h"

#include "GameEvaluatorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Common/Clusters/CrowdClusterTypes.h"
#include "MapAnalyzer/ClustersHashGrid.h"
#include "MapAnalyzer/MapAnalyzerSubsystem.h"


UAgentClusterProcessor::UAgentClusterProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UAgentClusterProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FClusterFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UAgentClusterProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	MapAnalyzerSubsystem = GetWorld()->GetSubsystem<UMapAnalyzerSubsystem>();
}

void UAgentClusterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UGameEvaluatorSubsystem* GameEvaluator       = GetWorld()->GetGameInstance()->GetSubsystem<UGameEvaluatorSubsystem>();
	ACrowdEvaluationHashGrid* EvaluationHashGrid = GameEvaluator->GetEvaluationHashGrid();
	
	UClustersHashGrid* ClustersHashGrid = MapAnalyzerSubsystem->GetClustersHashGrid();
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, ClustersHashGrid, EvaluationHashGrid](FMassExecutionContext& Context)
	{
		const int32 NumEntities                            = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FClusterFragment> ClusterList     = Context.GetMutableFragmentView<FClusterFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			FTransformFragment& TransformFragment = TransformList[EntityIndex];
			FClusterFragment& ClusterFragment     = ClusterList[EntityIndex];
			const FVector EntityLocation          = TransformFragment.GetMutableTransform().GetLocation();

			if (const FClusterCellData* ClusterData = ClustersHashGrid->GetClusterDataAtLocation(EntityLocation))
			{
				ClusterFragment.ClusterType    = ClusterData->ClusterType;
				ClusterFragment.PreviousAreaId = ClusterFragment.AreaId;
				ClusterFragment.AreaId         = EvaluationHashGrid->GetAreaIdAtLocation(EntityLocation);
			}
		}
	});
}
