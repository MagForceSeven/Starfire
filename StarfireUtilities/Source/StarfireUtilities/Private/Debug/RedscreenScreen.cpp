
#include "Debug/RedscreenScreen.h"

// UMG
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RedscreenScreen)

UTextBlock* URedscreenScreen::CreateText( const FString &Text ) const
{
	const auto TextBlock = WidgetTree->ConstructWidget< UTextBlock >( );

	TextBlock->SetText( FText::FromString( Text ) );

	return TextBlock;
}

void URedscreenScreen::CloseScreen_Implementation( )
{
	OnClose.Broadcast( this );
	
	RemoveFromParent( );
}
