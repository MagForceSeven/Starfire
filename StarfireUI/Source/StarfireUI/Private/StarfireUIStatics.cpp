
#include "StarfireUIStatics.h"

#include "StarfireHUD.h"
#include "StarfireHUDWidget.h"

// UMG
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"

// Engine
#include "EngineUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireUIStatics)

bool UStarfireUIStatics::IsMouseOverUI( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );

	const TActorIterator< AStarfireHUD > It( World );
	if (!It)
		return false;

	if (It->StarfireHUDWidget == nullptr)
		return false;

	return It->StarfireHUDWidget->IsMouseOverUI( );
}

UStarfireHUDWidget* UStarfireUIStatics::GetHUDWidget( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );

	const TActorIterator< AStarfireHUD > It( World );
	if (!It)
		return nullptr;

	return It->StarfireHUDWidget;
}

UWidget* UStarfireUIStatics::CreateNonUserWidget( TSubclassOf< UWidget > WidgetType, UObject *WorldContext )
{
	const auto WidgetOuter = Cast< UUserWidget >( WorldContext );
	if (!ensureAlways( WidgetOuter != nullptr ))
		return nullptr;

	return WidgetOuter->WidgetTree->ConstructWidget< UWidget >( WidgetType );
}