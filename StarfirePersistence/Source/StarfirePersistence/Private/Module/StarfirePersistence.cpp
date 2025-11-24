
#include "Module/StarfirePersistence.h"

#include "Modules/ModuleInterface.h"

DEFINE_LOG_CATEGORY( LogStarfirePersistence )

// UE module definition for runtime implementation of StarfirePersistence plugin
class FStarfirePersistence : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule( ) override;
	void ShutdownModule( ) override;
};

#define LOCTEXT_NAMESPACE "StarfirePersistence"

void FStarfirePersistence::StartupModule( )
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FStarfirePersistence::ShutdownModule( )
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FStarfirePersistence, StarfirePersistence )