
#include "StarfireHUD.h"

#include "StarfireHUDWidget.h"
#include "StarfireUILayout.h"
#include "StarfireScreen.h"

#include "Input/StarfireInputSubsystems.h"

// Modular Gameplay
#include "Components/GameFrameworkComponentManager.h"

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
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGameFrameworkComponentManager::NAME_GameActorReady );

	Super::BeginPlay( );
}

void AStarfireHUD::PreInitializeComponents( )
{
	Super::PreInitializeComponents( );

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver( this );
}

void AStarfireHUD::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver( this );

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
