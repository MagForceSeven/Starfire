
#include "StarfireHUD.h"

#include "StarfireHUDWidget.h"
#include "StarfireUILayout.h"
#include "StarfireScreen.h"

#include "Input/StarfireInputSubsystems.h"

// Common Input
#include "ICommonInputModule.h"

// Common UI
#include "Input/CommonUIInputTypes.h"

// Enhanced Input
#include "EnhancedInputComponent.h"

// Engine
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireHUD)

void AStarfireHUD::BeginPlay( void )
{
	Super::BeginPlay( );
}

void AStarfireHUD::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if (StarfireHUDWidget != nullptr)
	{
		UCommonUIExtensions::PopContentFromLayer( StarfireHUDWidget );
		StarfireHUDWidget = nullptr;
	}

	Super::EndPlay( EndPlayReason );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AStarfireHUD::OpenPauseMenu( )
{
	if (PauseMenuScreen == nullptr)
		return;

	UStarfireScreen::OpenNewScreen( this, UStarfireUILayout::LAYERTAG_MENU, PauseMenuScreen.Get( ) );
}

UStarfireHUDWidget* AStarfireHUD::CreateHUDWidget( const TSubclassOf< UStarfireHUDWidget > &WidgetType )
{
	check( StarfireHUDWidget == nullptr );
	
	const auto Layout = UStarfireUILayout::GetPrimaryGameLayout( GetOwningPlayerController(  ) );
	check( Layout != nullptr );

	StarfireHUDWidget = Layout->PushWidgetToLayerStack< UStarfireHUDWidget >( UStarfireUILayout::LAYERTAG_GAME, WidgetType );
	check( StarfireHUDWidget != nullptr );

	const auto LocalPlayer = PlayerOwner->GetLocalPlayer( );
	if (const auto InputSubsystem = ULocalPlayer::GetSubsystem< UStarfireInputLocalPlayerSubsystem >( LocalPlayer ))
	{
		InputSubsystem->AddMappingContext( DefaultMappingContext, 0 );
		InputSubsystem->PushMode( InitialInputMode );
	}

	PlayerOwner->GetComponentByClass< UEnhancedInputComponent >( )->BindAction( ICommonInputModule::GetSettings( ).GetEnhancedInputBackAction( ), ETriggerEvent::Triggered, this, &AStarfireHUD::OpenPauseMenu );

	return StarfireHUDWidget;
}
