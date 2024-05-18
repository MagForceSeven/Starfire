
#include "Debug/RedscreenScreen.h"

// UMG
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

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
