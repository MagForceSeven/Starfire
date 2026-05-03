
#pragma once

#include "PrimaryGameLayout.h"
#include "Blueprint/UserWidget.h"

#include "NativeGameplayTags_SF.h"

#include "StarfireUILayout.generated.h"

class UImage;
class UNegativeSpace;

// Starfire implementation for common elements to set up in a game layout widget
UCLASS( Blueprintable )
class STARFIREUI_API UStarfireUILayout : public UPrimaryGameLayout
{
	GENERATED_BODY( )
public:
	explicit UStarfireUILayout( const FObjectInitializer& ObjectInitializer );

	// Default layer IDs
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(LAYERTAG_GAME);
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(LAYERTAG_GAMEMENU);
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(LAYERTAG_MENU);
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(LAYERTAG_MODAL);

	// Overloads of Game Layout Accessors
	static UStarfireUILayout* GetPrimaryGameLayoutForPrimaryPlayer( const UObject *WorldContext );
	static UStarfireUILayout* GetPrimaryGameLayout( const UObject *WorldContext );
	static UStarfireUILayout* GetPrimaryGameLayout( const APlayerController *PlayerController );
	static UStarfireUILayout* GetPrimaryGameLayout( const ULocalPlayer *LocalPlayer );

	// User Widget API
	void NativeOnInitialized( ) override;
	void NativeConstruct( ) override;
	void NativeDestruct( ) override;

	// Get the full set of layer names in this layout
	TArray< FGameplayTag > GetLayerNames( void ) const { return LayerNames; }

	// The collection of layer stacks making up this layout
	TArray< UCommonActivatableWidgetContainerBase* > GetLayerWidgets( void );

	// Get the widget that's tracking the negative space behind all the layer stack widgets
	UNegativeSpace* GetNegativeSpace( void ) const { return NegativeSpace; }

	// Get the widget that should be used for applying fades to the viewport
	UImage* GetFadeOverlay( void ) const { return FaderImage; }

	// Hooks that can be used to respond to new layouts being created and added to a viewport
	DECLARE_MULTICAST_DELEGATE_OneParam( FLayoutChange, UStarfireUILayout* );
	static FLayoutChange OnLayoutAdded;
	static FLayoutChange OnLayoutRemoved;

protected:
	// The layer names to use for each stack of the layout
	UPROPERTY( EditDefaultsOnly, meta =(Categories = "UI.Layer") )
	TArray< FGameplayTag > LayerNames;

	// Negative space widget to capture mouse _not_ being over UI elements
	UPROPERTY( )
	TObjectPtr< UNegativeSpace > NegativeSpace = nullptr;

	// A widget that can be used to apply a fade to the screen on top of all the widgets that may have been added to the viewport
	UPROPERTY( )
	TObjectPtr< UImage > FaderImage = nullptr;
};

// A widget that can be placed below every other widget to figure out when the mouse is not over a UI element
UCLASS( )
class UNegativeSpace : public UUserWidget
{
	GENERATED_BODY( )
public:
	// Callbacks for changes to the mouse hover state of this widget
	DECLARE_DELEGATE( FHoverChanged );
	FHoverChanged OnHovered;
	FHoverChanged OnUnhovered;

	// User Widget API
	void NativeConstruct( ) override;

	void NativeOnMouseEnter( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	void NativeOnMouseLeave( const FPointerEvent& InMouseEvent ) override;

	FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

private:
	// Tracking mouse button being down over the widget
	// To handle what UMG thinks is click-and-drag cases
	bool bMouseIsDown = false;
	// Track mouse being over the widget
	bool bIsHovered = false;
};