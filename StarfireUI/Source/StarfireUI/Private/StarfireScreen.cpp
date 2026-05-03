
#include "StarfireScreen.h"

#include "StarfireUILayout.h"

// Gameplay Tags
#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireScreen)

UStarfireScreen::UStarfireScreen( )
{
	bIsBackHandler = true;
	InputConfig = EWidgetInputMode_SF::Menu;
}

UStarfireScreen* UStarfireScreen::OpenNewScreen( const UObject *WorldContext, FGameplayTag LayerName, TSoftClassPtr< UStarfireScreen > ScreenType )
{
	if (!ensureAlways(WorldContext != nullptr))
		return nullptr;
	if (!ensureAlways(LayerName.IsValid( )))
		return nullptr;
	if (!ensureAlways(!ScreenType.IsNull( )))
		return nullptr;

	const auto ScreenClass = ScreenType.LoadSynchronous( );
	if (!ensureAlways(ScreenClass != nullptr))
		return nullptr;

	const auto Layout = UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( WorldContext );
	if (!ensureAlways(Layout != nullptr))
		return nullptr;

	const auto Screen = Layout->PushWidgetToLayerStack< UStarfireScreen >( LayerName, ScreenClass );
	check(Screen != nullptr);

	Screen->HandleOnOpen( );

	return Screen;
}

void UStarfireScreen::NativeConstruct( )
{
	Super::NativeConstruct( );
}

void UStarfireScreen::NativeDestruct( )
{
	if (bIsOpen)
	{
		HandleOnClose( );
		OnClose.Broadcast( this );
	}

	Super::NativeDestruct( );
}

bool UStarfireScreen::NativeOnHandleBackAction( )
{
	if (!bIsOpen)
		return false;

	CloseScreen( );

	return true;
}

void UStarfireScreen::CloseScreen_Implementation( )
{
	bIsOpen = false;

	HandleOnClose( );
	OnClose.Broadcast( this );

	const auto Layout = UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( this );
	check( Layout != nullptr );

	Layout->FindAndRemoveWidgetFromLayer( this );
}

void UStarfireScreen::HandleOnClose_Implementation( )
{

}

void UStarfireScreen::HandleOnOpen_Implementation( )
{
	bIsOpen = true;
}