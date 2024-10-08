
#pragma once

#include "SaveData/SaveData.h"

#include "GameSaveData.generated.h"

// Game specific save game information
UCLASS( )
class SAVEDATAEXAMPLE_API UGameSaveData : public USaveData
{
	GENERATED_BODY( )
public:
	// Core Save Header API
	[[nodiscard]] bool IsCompatible( uint32 InVersion ) const override;

	// Apply this save game to the game data
	bool ApplySaveData( const UObject *WorldContext ) const;

	// The map that was loaded at the time and should be loaded into when loading the save
	UPROPERTY( )
	FSoftObjectPath MapPath;

	// Mode specific descriptor for UI display
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Data" )
	FText Descriptor1;

	// Mode specific descriptor for UI display
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Save Data" )
	FText Descriptor2;

	// Un-serialized flag tracking the (potentially async) completion
	bool bCreationComplete = false;
};