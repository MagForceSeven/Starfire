
#pragma once

#include "StarfireActivatableWidget.h"

#include "StarfireScreen.generated.h"

struct FGameplayTag;

// Intermediate widget class that defines common functionality for a "screen" that may be opened and closed dynamically
UCLASS( abstract, meta = (DisplayName = "Starfire Screen") )
class STARFIREUI_API UStarfireScreen : public UStarfireActivatableWidget
{
	GENERATED_BODY( )
public:
	UStarfireScreen( );
	
	// User Widget API
	void NativeConstruct( ) override;
	void NativeDestruct( ) override;

	// Common Activatable Widget
	bool NativeOnHandleBackAction( ) override;

	// Delegate type for the screen being closed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCloseEvent, UStarfireScreen*, Screen );

	// Dispatcher for an OnClose event from this screen
	UPROPERTY( BlueprintAssignable, Category = "ScreenEvents" )
	FOnCloseEvent OnClose;

	// Start the screen closing process
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "ScreenEvents" )
	void CloseScreen( );

	// Hook allowing screen to execute any initial processes
	UFUNCTION( BlueprintNativeEvent )
	void HandleOnOpen( );
	virtual void HandleOnOpen_Implementation( );

	// Create a new screen of a specified type and add it to the active canvas
	UFUNCTION( BlueprintCallable, Category = "Starfire UI", meta = (WorldContext = "WorldContext") )
	static UStarfireScreen* OpenNewScreen( const UObject *WorldContext, UPARAM( meta = (Categories = "UI.Layer") ) FGameplayTag LayerName, UPARAM( meta = (AllowAbstract = false) ) TSoftClassPtr< UStarfireScreen > ScreenType );

protected:
	// Hook allowing screen to execute any last changes required when being closed
	UFUNCTION( BlueprintNativeEvent )
	void HandleOnClose( );
	virtual void HandleOnClose_Implementation( );

	// Track the actual state of the screen which may not quite match its Construct/Destruct state
	UPROPERTY( BlueprintReadOnly )
	bool bIsOpen = false;
};