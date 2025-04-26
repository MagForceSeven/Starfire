
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "GameSaveDataSubsystem.generated.h"

class UGameSaveData;

// A game instance subsystem for use by the save system for various application duration tracking needs
UCLASS( )
class UGameSaveDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem Accessor Utilities
	[[nodiscard]] static UGameSaveDataSubsystem* Get( const UObject *WorldContext );
	[[nodiscard]] static UGameSaveDataSubsystem* Get( const UGameInstance *GameInstance );

	// The save data that should be used to populate game objects when the map is loaded
	UPROPERTY( )
	TObjectPtr< const UGameSaveData > SaveGame = nullptr;

	// The slot name of the last save that was loaded
	FString LastSaveSlotName;
};

// Configuration settings for project specific save data settings
UCLASS( Config = "Game", DefaultConfig )
class UGameSaveDataSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:
	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};