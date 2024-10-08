
#pragma once

#include "UObject/Object.h"

#include "SaveData.generated.h"

// Options available for modifying the Serialize To/From Bytes calls
struct FSerializeBytesOptions
{
	// Configures the 'WantsBinarySerialization' option of FArchive
	bool bWantBinary = false;

	// Configures the 'ArIsSaveGame' option of FArchive
	bool bSaveGame = false;
};

// Base class for save game data compatible with custom core save utilities
UCLASS( Abstract )
class STARFIRESAVEDATA_API USaveData : public UObject
{
	GENERATED_BODY( )
public:
	// The engine version at the time this save was created
	UPROPERTY( )
	uint32 PackageVersion = (uint32)EUnrealEngineObjectUE5Version::AUTOMATIC_VERSION;

	// Our custom version of the engine at the time this save was created
	UPROPERTY( )
	int32 LicenseeVersion = GPackageFileLicenseeUEVersion;

	// A custom version specific to the game
	UPROPERTY( )
	uint32 GameVersion = 0;

	// The content packages that were enabled for that play session
	UPROPERTY( )
	TArray< FString > ContentFeatures;

	// Fill the CoreSaveGame members with data for the specified world context
	void FillCoreData( const UObject *WorldContext, uint32 InGameVersion );

	// Hooks for save game version-ing information
	[[nodiscard]] virtual bool IsCompatible( uint32 InVersion ) const { return true; }

	// Configure an archive with all the applicable version information
	void ConfigureArchiveVersions( FArchive &Ar ) const;
	// Utility to convert an object into a collection of bytes
	void SerializeToBytes( UObject *Object, TArray< uint8 > &Bytes, const FSerializeBytesOptions &Options = { } ) const;
	// Utility to convert a collection of bytes back into an object
	void SerializeFromBytes( UObject *Object, const TArray< uint8 > &Bytes, const FSerializeBytesOptions &Options = { } ) const;

	// UObject API
	bool IsPostLoadThreadSafe( ) const override { return true; }
};