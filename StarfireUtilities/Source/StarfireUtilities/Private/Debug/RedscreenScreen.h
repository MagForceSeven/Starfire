
#pragma once

#include "Blueprint/UserWidget.h"

#include "RedscreenScreen.generated.h"

class UTextBlock;

// Screen type that can be used to display redscreen warnings
UCLASS( abstract, meta = (DisplayName = "Redscreen Presenter") )
class URedscreenScreen : public UUserWidget
{
	GENERATED_BODY( )
public:
	// Add a message to a
	UFUNCTION( BlueprintImplementableEvent )
	void AddMessage( const FString &Message );

	// Delegate type for the screen being closed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCloseEvent, URedscreenScreen*, Screen );

	// Dispatcher for an OnClose event from this screen
	UPROPERTY( BlueprintAssignable, Category = "Redscreen" )
	FOnCloseEvent OnClose;
	
	// Start the screen closing process
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "Redscreen" )
	void CloseScreen( );

protected:
	// Create a text widget that can be added to the widget tree (because blueprint can't dynamically create non-user widgets )
	UFUNCTION( BlueprintCallable, Category = "Redscreen", meta = (AutoCreateRefTerm = "Text") )
	UTextBlock* CreateText( const FString &Text ) const;
};