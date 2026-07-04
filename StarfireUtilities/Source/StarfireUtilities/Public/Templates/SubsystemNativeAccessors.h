
#pragma once

#include "Templates/TypeUtilitiesSF.h"

class UGameInstance;
class ULocalPlayer;

class UGameInstanceSubsystem;
class UWorldSubsystem;
class ULocalPlayerSubsystem;
class UEngineSubsystem;

#if WITH_EDITOR
	class UEditorSubsystem;
#endif

// Native utilities that wrap accessors to the subsystem outers as well as the subsystems themselves
// Avoids expensive includes required to access needed utilities
struct STARFIREUTILITIES_API FSubsystemNativeAccessorsBase
{
protected:
	[[nodiscard]] static const UWorld* GetWorldFromContext( const UObject *WorldContext );
	[[nodiscard]] static const UGameInstance* GetGameInstanceFromContext( const UObject *WorldContext );
	[[nodiscard]] static const ULocalPlayer* GetLocalPlayerFromContext( const UObject *WorldContext );

	[[nodiscard]] static UWorldSubsystem* GetWorldSubsystem( const UWorld *World, const TSubclassOf< UWorldSubsystem > &SubsystemType );
	[[nodiscard]] static UGameInstanceSubsystem* GetGameInstanceSubsystem( const UGameInstance *GameInstance, const TSubclassOf< UGameInstanceSubsystem > &SubsystemType );

	[[nodiscard]] static ULocalPlayerSubsystem* GetLocalPlayerSubsystem( const ULocalPlayer *LocalPlayer, const TSubclassOf< ULocalPlayerSubsystem > &SubsystemType );
	[[nodiscard]] static ULocalPlayerSubsystem* GetLocalPlayerSubsystem( const APlayerController *Controller, const TSubclassOf< ULocalPlayerSubsystem > &SubsystemType );

	[[nodiscard]] static UEngineSubsystem* GetEngineSubsystem( const TSubclassOf< UEngineSubsystem > &SubsystemType );

#if WITH_EDITOR
	[[nodiscard]] static UEditorSubsystem* GetEditorSubsystem( const TSubclassOf< UEditorSubsystem > &SubsystemType );
#endif
};

// Mixin template CRTP that adds static GetSubsystem member functions based on the type of subsystem
// For example, for a GameInstanceSubsystem functions support a const UObject* or const UGameInstance* input
// while for a WorldSubsystem functions support a const UObject* or const UWorld* input
template < class type_t >
struct TSubsystemNativeAccessors : public FSubsystemNativeAccessorsBase
{
	[[nodiscard]] static type_t* GetSubsystem( const UWorld *World ) requires SFstd::derived_from< type_t, UWorldSubsystem >
	{
		return CastChecked< type_t >( GetWorldSubsystem( World, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
	
	[[nodiscard]] static type_t* GetSubsystem( const UObject *WorldContext ) requires SFstd::derived_from< type_t, UWorldSubsystem >
	{
		const auto World = GetWorldFromContext( WorldContext );
		return GetSubsystem( World );
	}

	[[nodiscard]] static type_t* GetSubsystem( const UGameInstance *GameInstance ) requires SFstd::derived_from< type_t, UGameInstanceSubsystem >
	{
		return CastChecked< type_t >( GetGameInstanceSubsystem( GameInstance, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
	
	[[nodiscard]] static type_t* GetSubsystem( const UObject *WorldContext ) requires SFstd::derived_from< type_t, UGameInstanceSubsystem >
	{
		const auto GameInstance = GetGameInstanceFromContext( WorldContext );
		return GetSubsystem( GameInstance );
	}

	[[nodiscard]] static type_t* GetSubsystem( ) requires SFstd::derived_from< type_t, UEngineSubsystem >
	{
		return CastChecked< type_t >( GetEngineSubsystem( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
	
	[[nodiscard]] static type_t* GetSubsystem( const UObject *WorldContext ) requires SFstd::derived_from< type_t, ULocalPlayerSubsystem >
	{
		const auto LocalPlayer = GetLocalPlayerFromContext( WorldContext );
		return GetSubsystem( LocalPlayer );
	}

	[[nodiscard]] static type_t* GetSubsystem( const APlayerController *PlayerController ) requires SFstd::derived_from< type_t, ULocalPlayerSubsystem >
	{
		return CastChecked< type_t >( GetLocalPlayerSubsystem( PlayerController, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
	
	[[nodiscard]] static type_t* GetSubsystem( const ULocalPlayer *LocalPlayer ) requires SFstd::derived_from< type_t, ULocalPlayerSubsystem >
	{
		return CastChecked< type_t >( GetLocalPlayerSubsystem( LocalPlayer, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}

#if WITH_EDITOR
	[[nodiscard]] static type_t* GetSubsystem( ) requires SFstd::derived_from< type_t, UEditorSubsystem >
	{
		return CastChecked< type_t >( GetEditorSubsystem( type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
#endif
};