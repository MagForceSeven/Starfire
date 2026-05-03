
#include "Widgets/StarfireButton.h"

// Engine
#include "Engine/Texture2D.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireButton)

#define LOCTEXT_NAMESPACE "StarfireButton"

UStarfireButton::UStarfireButton( )
{
	OnClicked.AddDynamic( this, &UStarfireButton::ButtonClicked );

	OnPressed.AddDynamic( this, &UStarfireButton::ButtonPressed );

	OnReleased.AddDynamic( this, &UStarfireButton::ButtonReleased );

	OnHovered.AddDynamic( this, &UStarfireButton::ButtonHover );

	OnUnhovered.AddDynamic( this, &UStarfireButton::ButtonUnhover );

	auto Style = GetStyle( );
	Style.Disabled.TintColor = FLinearColor( 1.0f, 1.0f, 1.0f, 0.5f );
	SetStyle( Style );
}

#if WITH_EDITOR
// ReSharper disable once CppConstValueFunctionReturnType
const FText UStarfireButton::GetPaletteCategory( )
{
	return LOCTEXT( "StarfireUI", "StarfireUI" );
}
#endif

void UStarfireButton::ButtonClicked( )
{
	OnClickedSF.Broadcast( this );
}

void UStarfireButton::ButtonPressed( )
{
	OnPressedSF.Broadcast( this );
}

void UStarfireButton::ButtonReleased( )
{
	OnReleasedSF.Broadcast( this );
}

void UStarfireButton::ButtonHover( )
{
	OnHoveredSF.Broadcast( this );
}

void UStarfireButton::ButtonUnhover( )
{
	OnUnhoveredSF.Broadcast( this );
}

void UStarfireButton::UpdateImage( UTexture2D *NewImage )
{
	auto Style = GetStyle( );

	Style.Normal.SetResourceObject( NewImage );
	Style.Disabled.SetResourceObject( NewImage );
	Style.Hovered.SetResourceObject( NewImage );
	Style.Pressed.SetResourceObject( NewImage );

	Style.Normal.DrawAs = ESlateBrushDrawType::Image;
	Style.Disabled.DrawAs = ESlateBrushDrawType::Image;
	Style.Hovered.DrawAs = ESlateBrushDrawType::Image;
	Style.Pressed.DrawAs = ESlateBrushDrawType::Image;

	SetStyle( Style );
}

void UStarfireButton::UpdateTint( FLinearColor NewColor )
{
	auto Style = GetStyle( );
	
	Style.Normal.TintColor = NewColor;
	Style.Hovered.TintColor = NewColor;
	Style.Pressed.TintColor = NewColor;

	NewColor.A = 0.5f;
	Style.Disabled.TintColor = NewColor;
	
	SetStyle( Style );
}

#undef LOCTEXT_NAMESPACE