
#pragma once

#include "UObject/Object.h"

#include "SaveDataHeader.generated.h"

// Epic uses FNames for compression ids which isn't as useful when we want to store it in a POD struct
enum class ESaveDataCompressionType : uint32
{
	None = 0,
	ZLib,
	GZip,
	Oodle,
};

class USaveData;

// Base class for header information compatible with core save game utilities
UCLASS( Abstract )
class STARFIRESAVEDATA_API USaveDataHeader : public UObject
{
	GENERATED_BODY( )
public:
	// Time that the save was created
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Header" )
	FDateTime TimeStamp;

	// The build that was used to create this save
	UPROPERTY( VisibleAnywhere, Category = "Save Header" )
	uint32 Changelist;
	
	// The version of the engine that was being used at the CL
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Header" )
	FString BuildVersion;

	// The language the user was playing in when they created this save.
	// If this does not match the game's current language, we display a fallback save name
	// As we may not have the fonts needed to display the characters that were actually used.
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Header" )
	FString Language;

	// The name to use as an alternate display name in UI
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Header" )
	FString DisplayName;

	// The content packages that were enabled for that play session
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Header" )
	TArray< FString > ContentFeatures;

	// Fill the CoreSaveHeader members with data for the specified save game data
	void FillCoreData( const USaveData *SaveData, const FString &InDisplayName );

	// Hooks for file version-ing support
	UE_NODISCARD virtual uint32 GetVersion( void ) const PURE_VIRTUAL( GetVersion, return -1; );
	UE_NODISCARD virtual bool IsCompatible( uint32 HeaderVersion ) const { return true; }
	UE_NODISCARD virtual int32 GetFileTypeTag( void ) const;

	// Hook for controlling the compression applied when saving the file
	UE_NODISCARD virtual ESaveDataCompressionType GetCompressionType( void ) const;
};