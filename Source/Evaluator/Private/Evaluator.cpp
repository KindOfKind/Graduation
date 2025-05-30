#include "Evaluator.h"

DEFINE_LOG_CATEGORY(Evaluator);

#define LOCTEXT_NAMESPACE "FEvaluator"

void FEvaluator::StartupModule()
{
	UE_LOG(Evaluator, Warning, TEXT("Evaluator module has been loaded"));
}

void FEvaluator::ShutdownModule()
{
	UE_LOG(Evaluator, Warning, TEXT("Evaluator module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEvaluator, Evaluator)