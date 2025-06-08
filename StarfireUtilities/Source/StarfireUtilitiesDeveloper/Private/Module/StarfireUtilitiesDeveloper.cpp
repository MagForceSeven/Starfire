
#include "Module/StarfireUtilitiesDeveloper.h"

#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "StarfireUtilitiesDeveloper"

void FStarfireUtilitiesDeveloper::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Create redirectors for the map and set for-each nodes to start using the ones now available from Epic
	TArray<FCoreRedirect> NewRedirects;

	NewRedirects.Emplace(ECoreRedirectFlags::Type_Class, TEXT("/Script/StarfireUtilitiesDeveloper.K2Node_MapForEach"), TEXT("/Script/BlueprintGraph.K2Node_MapForEach"));
	NewRedirects.Emplace(ECoreRedirectFlags::Type_Class, TEXT("/Script/StarfireUtilitiesDeveloper.K2Node_SetForEach"), TEXT("/Script/BlueprintGraph.K2Node_SetForEach"));

	FCoreRedirects::AddRedirectList(NewRedirects, TEXT("StarfireUtilities"));
}

void FStarfireUtilitiesDeveloper::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfireUtilitiesDeveloper, StarfireUtilitiesDeveloper )