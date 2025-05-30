#include "Navigation.h"

DEFINE_LOG_CATEGORY(Navigation);

#define LOCTEXT_NAMESPACE "FNavigation"

void FNavigation::StartupModule()
{
	UE_LOG(Navigation, Warning, TEXT("Navigation module has been loaded"));
}

void FNavigation::ShutdownModule()
{
	UE_LOG(Navigation, Warning, TEXT("Navigation module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNavigation, Navigation)