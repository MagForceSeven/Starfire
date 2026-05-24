
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "GameSaveDataSubsystem.generated.h"

class UGameSaveData;

// A game instance subsystem for use by the save system for various application duration tracking needs
UCLASS( )
class SAVEDATAEXAMPLE_API UGameSaveDataSubsystem : public UGameInstanceSubsystem, public TSubsystemNativeAccessors< UGameSaveDataSubsystem >
{
	GENERATED_BODY( )
public:
	// The save data that should be used to populate game objects when the map is loaded
	UPROPERTY( )
	TObjectPtr< const UGameSaveData > SaveGame = nullptr;

	// The slot name of the last save that was loaded
	FString LastSaveSlotName;
	
	// Subsystem API
	void Initialize( FSubsystemCollectionBase& Collection ) override;
	void Deinitialize( ) override;

private:
	// Handle the transition from one world to the next
	UFUNCTION( )
	void HandleNewWorld( UGameInstance *GameInstance, UWorld *OldWorld, UWorld *NewWorld );

	// Handle the start of gameplay in worlds
	UFUNCTION( )
	void HandleWorldBeginPlay( bool bBeginPlay );
};

// Configuration settings for project specific save data settings
UCLASS( Config = "Game", DefaultConfig )
class SAVEDATAEXAMPLE_API UGameSaveDataSettings : public UDeveloperSettings
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