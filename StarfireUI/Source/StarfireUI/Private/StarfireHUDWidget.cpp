
#include "StarfireHUDWidget.h"

#include "StarfireUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireHUDWidget)

UStarfireHUDWidget::UStarfireHUDWidget( )
{
	InputConfig = EWidgetInputMode_SF::GameAndMenu;
}

void UStarfireHUDWidget::NativeConstruct( )
{
	Super::NativeConstruct( );

	const auto Layout = UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( this );
	const auto NegativeSpace = Layout->GetNegativeSpace( );
	
	// Hook up some callback for the hover state changing
	NegativeSpace->OnHovered.BindUObject( this, &UStarfireHUDWidget::NegativeSpace_OnHovered );
	NegativeSpace->OnUnhovered.BindUObject( this, &UStarfireHUDWidget::NegativeSpace_OnUnhovered );
}

void UStarfireHUDWidget::NativeDestruct( )
{
	const auto Layout = UStarfireUILayout::GetPrimaryGameLayoutForPrimaryPlayer( this );
	const auto NegativeSpace = Layout->GetNegativeSpace( );

	NegativeSpace->OnHovered.Unbind( );
	NegativeSpace->OnUnhovered.Unbind( );
	
	Super::NativeDestruct( );
}

void UStarfireHUDWidget::NegativeSpace_OnHovered( )
{
	bOverNegativeSpace = true;
	//GEngine->AddOnScreenDebugMessage( INDEX_NONE, 5.0f, FColor::Green, TEXT( "Negative Space Hovered" ) );
}

void UStarfireHUDWidget::NegativeSpace_OnUnhovered( )
{
	bOverNegativeSpace = false;
	//GEngine->AddOnScreenDebugMessage( INDEX_NONE, 5.0f, FColor::Green, TEXT( "Negative Space Unhovered" ) );
}
