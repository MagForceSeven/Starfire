
#include "StarfireUILayout.h"

// UMG
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireUILayout)

UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireUILayout::LAYERTAG_GAME, "UI.Layer.Game", "Primary Game UI" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireUILayout::LAYERTAG_GAMEMENU, "UI.Layer.GameMenu", "Gameplay Menus" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireUILayout::LAYERTAG_MENU, "UI.Layer.Menu", "Other application menus" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireUILayout::LAYERTAG_MODAL, "UI.Layer.Modal", "Dialog boxes" );

UStarfireUILayout::FLayoutChange UStarfireUILayout::OnLayoutAdded;
UStarfireUILayout::FLayoutChange UStarfireUILayout::OnLayoutRemoved;

UStarfireUILayout::UStarfireUILayout( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	// Default the layer names to a reasonable default
	LayerNames.Push( LAYERTAG_GAME );
	LayerNames.Push( LAYERTAG_GAMEMENU );
	LayerNames.Push( LAYERTAG_MENU );
	LayerNames.Push( LAYERTAG_MODAL );
}

UStarfireUILayout* UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( const UObject *WorldContext )
{
	return CastChecked< UStarfireUILayout >( Super::GetPrimaryGameLayoutForPrimaryPlayer( WorldContext ) );
}

UStarfireUILayout* UStarfireUILayout::GetPrimaryGameLayout( const UObject *WorldContext )
{
	if (const auto Controller = Cast< APlayerController >( WorldContext ))
		return GetPrimaryGameLayout( Controller );

	if (const auto Player = Cast< ULocalPlayer >( WorldContext ))
		return GetPrimaryGameLayout( Player );
	
	return GetPrimaryGameLayoutForPrimaryPlayer( WorldContext );
}

UStarfireUILayout* UStarfireUILayout::GetPrimaryGameLayout( const APlayerController *PlayerController )
{
	return CastChecked< UStarfireUILayout >( Super::GetPrimaryGameLayout( const_cast< APlayerController* >( PlayerController ) ) );
}

UStarfireUILayout* UStarfireUILayout::GetPrimaryGameLayout( const ULocalPlayer *LocalPlayer )
{
	return CastChecked< UStarfireUILayout >( Super::GetPrimaryGameLayout( const_cast< ULocalPlayer* >( LocalPlayer ) ) );
}

void UStarfireUILayout::NativeOnInitialized( )
{
	Super::NativeOnInitialized( );

	WidgetTree->RootWidget = WidgetTree->ConstructWidget< UOverlay >(  );
}

void UStarfireUILayout::NativeConstruct( )
{
	Super::NativeConstruct( );

	const auto Overlay = CastChecked< UOverlay >( WidgetTree->RootWidget );

	// Place a negative space widget behind all the widget stacks
	{
		NegativeSpace = WidgetTree->ConstructWidget< UNegativeSpace >( );
		const auto OverlaySlot = Overlay->AddChildToOverlay( NegativeSpace );

		OverlaySlot->SetHorizontalAlignment( EHorizontalAlignment::HAlign_Fill );
		OverlaySlot->SetVerticalAlignment( EVerticalAlignment::VAlign_Fill );
		OverlaySlot->SetPadding( FMargin( ) );
	}

	// Spawn widget stacks for each of the configured layer tags
	for (const auto &LayerName : LayerNames)
	{
		const auto Container = WidgetTree->ConstructWidget< UCommonActivatableWidgetStack >( );
		const auto OverlaySlot = Overlay->AddChildToOverlay( Container );

		OverlaySlot->SetHorizontalAlignment( EHorizontalAlignment::HAlign_Fill );
		OverlaySlot->SetVerticalAlignment( EVerticalAlignment::VAlign_Fill );
		OverlaySlot->SetPadding( FMargin( ) );

		RegisterLayer( LayerName, Container );
	}

	// Place an image on top
	{
		FaderImage = WidgetTree->ConstructWidget< UImage >( );
		const auto OverlaySlot = Overlay->AddChildToOverlay( FaderImage );

		FaderImage->SetVisibility( ESlateVisibility::HitTestInvisible );
		FaderImage->SetRenderOpacity( 0.0f );

		OverlaySlot->SetHorizontalAlignment( EHorizontalAlignment::HAlign_Fill );
		OverlaySlot->SetVerticalAlignment( EVerticalAlignment::VAlign_Fill );
		OverlaySlot->SetPadding( FMargin( ) );
	}

	OnLayoutAdded.Broadcast( this );
}

void UStarfireUILayout::NativeDestruct( )
{
	OnLayoutRemoved.Broadcast( this );
	
	Super::NativeDestruct( );
}

TArray< UCommonActivatableWidgetContainerBase* > UStarfireUILayout::GetLayerWidgets( )
{
	TArray< UCommonActivatableWidgetContainerBase* > Widgets;

	for (const auto &LayerName : LayerNames)
	{
		if (const auto Layer = GetLayerWidget(LayerName))
			Widgets.Push( Layer );
	}

	return Widgets;
}

//**********************************************************************************************************************
//			Negative Space Widget
//**********************************************************************************************************************

void UNegativeSpace::NativeConstruct( )
{
	Super::NativeConstruct( );

	SetVisibility( ESlateVisibility::Visible );
}

void UNegativeSpace::NativeOnMouseEnter( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	Super::NativeOnMouseEnter( InGeometry, InMouseEvent );

	bMouseIsDown = false;

	if (!bIsHovered)
	{
		bIsHovered = true;
		OnHovered.ExecuteIfBound( );
	}
}

void UNegativeSpace::NativeOnMouseLeave( const FPointerEvent& InMouseEvent )
{
	Super::NativeOnMouseLeave( InMouseEvent );

	if (!bMouseIsDown && bIsHovered)
	{
		bIsHovered = false;
		OnUnhovered.ExecuteIfBound( );
	}
}

FReply UNegativeSpace::NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	bMouseIsDown = false;
	return FReply::Unhandled( );
}

FReply UNegativeSpace::NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	bMouseIsDown = true;
	return FReply::Unhandled( );
}