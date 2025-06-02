#include "ORCA.h"

DEFINE_LOG_CATEGORY(ORCA);

#define LOCTEXT_NAMESPACE "FORCA"

void FORCA::StartupModule()
{
	UE_LOG(ORCA, Warning, TEXT("ORCA module has been loaded"));
}

void FORCA::ShutdownModule()
{
	UE_LOG(ORCA, Warning, TEXT("ORCA module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FORCA, ORCA)