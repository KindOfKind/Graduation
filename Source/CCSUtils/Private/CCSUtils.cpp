#include "CCSUtils.h"

DEFINE_LOG_CATEGORY(CCSUtils);

#define LOCTEXT_NAMESPACE "FCCSUtils"

void FCCSUtils::StartupModule()
{
	UE_LOG(CCSUtils, Warning, TEXT("CCSUtils module has been loaded"));
}

void FCCSUtils::ShutdownModule()
{
	UE_LOG(CCSUtils, Warning, TEXT("CCSUtils module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCCSUtils, CCSUtils)