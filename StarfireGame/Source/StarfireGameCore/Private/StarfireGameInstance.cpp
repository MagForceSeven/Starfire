
#include "StarfireGameInstance.h"

#include "DataDefinitions/DataDefinitionLibrary.h"

#include "Misc/DevSettingsPreloader.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireGameInstance)

void UStarfireGameInstance::Init( )
{
	Super::Init( );

	// Loading of an always loaded bundle for content that should be loaded for the duration
	const auto Library = UDataDefinitionLibrary::GetInstance( );

	const auto LoadAlwaysLoaded = Library->GameInstanceInit( this );
	const auto LoadDevSettingsPreload = IDevSettingsPreloader::PreloadAll( this );

	// Temporarily disable this streaming frame delay so that the completion callback isn't shifted to Tick
	static auto StreamableDelegateDelay = IConsoleManager::Get( ).FindConsoleVariable( TEXT("s.StreamableDelegateDelayFrames") );
	check( StreamableDelegateDelay != nullptr );

	const auto CurrentFrameDelay = StreamableDelegateDelay->GetInt( );
	StreamableDelegateDelay->Set( 0 );

	if (LoadAlwaysLoaded.IsValid( ))
		LoadAlwaysLoaded->WaitUntilComplete( );
	if (LoadDevSettingsPreload.IsValid( ))
		LoadDevSettingsPreload->WaitUntilComplete( );

	// Restore the value back to the default
	StreamableDelegateDelay->Set( CurrentFrameDelay );
}

void UStarfireGameInstance::Shutdown( )
{
	IDevSettingsPreloader::GameShutdown( this );
	
	// Unloading of the always loaded bundle 
	const auto Library = UDataDefinitionLibrary::GetInstance( );
	Library->GameInstanceShutdown( this );

	Super::Shutdown( );
}