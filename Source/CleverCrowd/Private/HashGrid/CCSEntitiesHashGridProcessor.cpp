// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGrid/CCSEntitiesHashGridProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Grids/GridUtilsFunctionLibrary.h"
#include "HashGrid/CCSEntitiesHashGrid.h"
#include "Management/CCSEntitiesManagerSubsystem.h"


UCCSEntitiesHashGridProcessor::UCCSEntitiesHashGridProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	bRequiresGameThreadExecution = true;
}

void UCCSEntitiesHashGridProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.RegisterWithProcessor(*this);
}

void UCCSEntitiesHashGridProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	EntitiesManager = GetWorld()->GetSubsystem<UCCSEntitiesManagerSubsystem>();
}

void UCCSEntitiesHashGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> Entities;
	UCCSEntitiesHashGrid* HashGrid = EntitiesManager->GetEntitiesHashGrid();
	check(HashGrid);
	HashGrid->DataLock.Lock();
	HashGrid->ClearData();
	
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, HashGrid](FMassExecutionContext& Context)
	{
		const int32 NumEntities                            = Context.GetNumEntities();
		const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FAgentRadiusFragment> RadiusList  = Context.GetMutableFragmentView<FAgentRadiusFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
		{
			const FVector& EntityLocation = TransformList[EntityIndex].GetTransform().GetLocation();
			HashGrid->AddEntityAtLocation(EntityLocation, Context.GetEntity(EntityIndex), RadiusList[EntityIndex].Radius);
		}
	});

	// HashGrid->ForEachNonEmptyCell([this, &EntityManager](const FGridCellPosition& Cell)
	// {
	// 	const FVector CellLoc = UGridUtilsFunctionLibrary::GetGridCellLocationAtPosition(Cell, 100);
	// 	DrawDebugLine(GetWorld(), CellLoc, CellLoc + FVector::UpVector * 500.f, FColor::Red, false, 0.f, 0, 2.f);
	// });
	
	HashGrid->DataLock.Unlock();
}

