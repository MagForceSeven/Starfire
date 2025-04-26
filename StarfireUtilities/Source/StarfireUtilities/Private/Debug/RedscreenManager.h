
#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "RedscreenManager.generated.h"

class UScreenBase;
class URedscreenScreen;

UCLASS( notplaceable, NotBlueprintable, Config = Redscreens )
class URedscreenManager final : public UWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Singleton Accessor
	static URedscreenManager* GetSubsystem( const UObject *WorldContext );

	// Add a message to the redscreen UI (possibly creating a screen if one is not active)
	void AddRedscreen( const FString &Message, bool bOnce );

	// Hook for the match having actually started (so that we know BeginPlay has been dispatched to everyone)
	void OnMatchStarted( const UObject *WorldContext );
	
	// world Subsystem API
	bool DoesSupportWorldType( const EWorldType::Type WorldType ) const override;
	// Subsystem API
	void Initialize(FSubsystemCollectionBase &Collection) override;
	void Deinitialize( ) override;

protected:
	// The type of screen that should be created that can display redscreen messages
	UPROPERTY( Config )
	TSoftClassPtr< URedscreenScreen > RedscreenWidgetType;

	// A reference to an active screen that is displaying messages
	UPROPERTY( )
	TObjectPtr<URedscreenScreen> ScreenWidget = nullptr;

	// A hook to be notified when the screen widget is closed (removed from the view)
	UFUNCTION( )
	void OnScreenClosed( URedscreenScreen *Screen );

	// Whether or not the screen is currently visible
	bool bVisible = false;

	// Messages that should be shown the next time a redscreen manager is available
	TArray< FString > QueuedMessages;

	// CRCs for the error messages that have been seen so that they're only seen once
	TSet< uint32 > SeenOnceCRCs;
};