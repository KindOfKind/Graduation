#include "CleverCrowdNavigator.h"

DEFINE_LOG_CATEGORY(CleverCrowdNavigator);

#define LOCTEXT_NAMESPACE "FCleverCrowdNavigator"

void FCleverCrowdNavigator::StartupModule()
{
	UE_LOG(CleverCrowdNavigator, Warning, TEXT("CleverCrowdNavigator module has been loaded"));
}

void FCleverCrowdNavigator::ShutdownModule()
{
	UE_LOG(CleverCrowdNavigator, Warning, TEXT("CleverCrowdNavigator module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleverCrowdNavigator, CleverCrowdNavigator)