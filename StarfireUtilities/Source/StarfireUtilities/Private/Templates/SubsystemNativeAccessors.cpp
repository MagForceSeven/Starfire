
#include "Templates/SubsystemNativeAccessors.h"

// UMG
#include "Components/Widget.h"

// Engine
#include "Kismet/GameplayStatics.h"

const UWorld* FSubsystemNativeAccessorsBase::GetWorldFromContext( const UObject *WorldContext )
{
	if (!IsValid( WorldContext ))
		return nullptr;

	return GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
}

const UGameInstance* FSubsystemNativeAccessorsBase::GetGameInstanceFromContext( const UObject *WorldContext )
{
	if (!IsValid( WorldContext ))
		return nullptr;

	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (World == nullptr)
		return nullptr;

	return World->GetGameInstance( );
}

const ULocalPlayer* FSubsystemNativeAccessorsBase::GetLocalPlayerFromContext( const UObject *WorldContext )
{
	if (const auto Player = Cast< ULocalPlayer >( WorldContext ))
		return Player;

	if (const auto Controller = Cast< APlayerController >( WorldContext ))
		return Controller->GetLocalPlayer( );

	if (const auto Pawn = Cast< APawn >( WorldContext ))
		return GetLocalPlayerFromContext( Pawn->GetController( ) );

	if (const auto Widget = Cast< UWidget >( WorldContext ))
		return Widget->GetOwningLocalPlayer( );

	const auto FirstController = UGameplayStatics::GetPlayerController( WorldContext, 0 );
	return FirstController->GetLocalPlayer( );
}

UWorldSubsystem* FSubsystemNativeAccessorsBase::GetWorldSubsystem( const UWorld *World, const TSubclassOf< UWorldSubsystem > &SubsystemType )
{
	check( SubsystemType != nullptr );

	if (!IsValid( World ))
		return nullptr;

	return World->GetSubsystemBase( SubsystemType );
}

UGameInstanceSubsystem* FSubsystemNativeAccessorsBase::GetGameInstanceSubsystem( const UGameInstance *GameInstance, const TSubclassOf< UGameInstanceSubsystem > &SubsystemType )
{
	check( SubsystemType != nullptr );

	if (!IsValid( GameInstance ))
		return nullptr;

	return GameInstance->GetSubsystemBase( SubsystemType );
}

ULocalPlayerSubsystem* FSubsystemNativeAccessorsBase::GetLocalPlayerSubsystem( const ULocalPlayer *LocalPlayer, const TSubclassOf< ULocalPlayerSubsystem > &SubsystemType )
{
	check( SubsystemType != nullptr );

	if (!IsValid( LocalPlayer ))
		return nullptr;

	return LocalPlayer->GetSubsystemBase( SubsystemType );
}

ULocalPlayerSubsystem * FSubsystemNativeAccessorsBase::GetLocalPlayerSubsystem( const APlayerController *Controller, const TSubclassOf<ULocalPlayerSubsystem> &SubsystemType )
{
	check( SubsystemType != nullptr );

	if (!IsValid( Controller ))
		return nullptr;

	return GetLocalPlayerSubsystem( Controller->GetLocalPlayer( ), SubsystemType );
}

UEngineSubsystem* FSubsystemNativeAccessorsBase::GetEngineSubsystem( const TSubclassOf< UEngineSubsystem > &SubsystemType )
{
	check( SubsystemType != nullptr );
	check( GEngine != nullptr );
	
	return GEngine->GetEngineSubsystemBase( SubsystemType );
}

#if WITH_EDITOR
UEditorSubsystem* FSubsystemNativeAccessorsBase::GetEditorSubsystem( const TSubclassOf< UEditorSubsystem > &SubsystemType )
{
	check( SubsystemType != nullptr );
	check( GEditor != nullptr );
	
	return GEditor->GetEditorSubsystemBase( SubsystemType );
}
#endif