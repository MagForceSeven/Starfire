
#include "LevelMetadata.h"

#include "AssetValidation/AssetChecks.h"

// Core UObject
#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LevelMetadata)

#define LOCTEXT_NAMESPACE "LevelMetadata"

void ULevelMetadata::Verify( const UObject *WorldContext )
{
	Super::Verify( WorldContext );

	if (Level.IsNull( ))
		AssetChecks::AC_Message( this, TEXT( "Level Metadata not configured with a valid level reference. Metadata asset types should not be created manually." ), WorldContext );
}

#if WITH_EDITOR
void ULevelMetadata::InitializeMetadata( const UWorld *World )
{
	if (!ensureAlways( IsValid( World ) ))
		return;

	Level = World;
}
#endif

#undef LOCTEXT_NAMESPACE