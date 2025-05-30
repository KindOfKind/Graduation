#include "CleverCrowd.h"

DEFINE_LOG_CATEGORY(CleverCrowd);

#define LOCTEXT_NAMESPACE "FCleverCrowd"

void FCleverCrowd::StartupModule()
{
	UE_LOG(CleverCrowd, Warning, TEXT("CleverCrowd module has been loaded"));
}

void FCleverCrowd::ShutdownModule()
{
	UE_LOG(CleverCrowd, Warning, TEXT("CleverCrowd module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCleverCrowd, CleverCrowd)