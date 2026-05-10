
#pragma once

#include "DataDefinitions/DataDefinition.h"

#include "LevelMetadata.generated.h"

// Base class for data definitions that can function as metadata for certain levels.
// The derived type defines naming and how to decide if a particular level needs that metadata information.
UCLASS( Abstract, NotBlueprintable )
class LEVELMETADATA_API ULevelMetadata : public UDataDefinition
{
	GENERATED_BODY( )
public:
	// Accessor to the level associated with this metadata
	[[nodiscard]] TSoftObjectPtr< const UWorld > GetLevel( void ) const { return Level; }

	// Verifiable Asset API
	void Verify( const UObject *WorldContext ) override;

protected:
#if WITH_EDITOR
	friend class FLevelMetadataEditor;

	// Hook for deciding if a particular level requires metadata of the derived type
	[[nodiscard]] virtual bool ShouldCreateMetadata( const UWorld *World ) const { return false; }

	// Hook for filling out the metadata based on the contents of the associated level
	virtual void InitializeMetadata( const UWorld *World );

	// A prefix to apply to the level name to create a unique package name for the metadata
	// At least one of the prefix or suffix must be non-empty
	[[nodiscard]] virtual FString GetMetadataPrefix( void ) const { return { }; }

	// A suffix to apply to the level name to create a unique package name for the metadata
	// At least one of the prefix or suffix must be non-empty
	[[nodiscard]] virtual FString GetMetadataSuffix( void ) const { return TEXT("_Metadata"); }
#endif

private:
	// The level associated with this metadata
	UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess = true) )
	TSoftObjectPtr< const UWorld > Level;
};