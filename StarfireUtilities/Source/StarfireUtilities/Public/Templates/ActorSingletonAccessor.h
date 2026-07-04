
#pragma once

#include "Templates/TypeUtilitiesSF.h"

// Native utilities that wrap accessors to worlds and actors
// Avoids expensive includes required to access needed utilities
struct STARFIREUTILITIES_API FActorSingletonAccessorsBase
{
protected:
	[[nodiscard]] static const UWorld* GetWorldFromContext( const UObject *WorldContext );

	[[nodiscard]] static AActor* GetActorSingleton( const UWorld *World, const TSubclassOf< AActor > &SingletonType );
};

// Mixin template CRTP that adds static GetSingleton member functions to the class for simpler access
template < class type_t >
struct TActorSingletonAccessors : public FActorSingletonAccessorsBase
{
	[[nodiscard]] static type_t* GetSingleton( const UWorld *World ) requires SFstd::derived_from< type_t, AActor >
	{
		return CastChecked< type_t >( GetActorSingleton( World, type_t::StaticClass( ) ), ECastCheckedType::NullAllowed );
	}
	
	[[nodiscard]] static type_t* GetSingleton( const UObject *WorldContext ) requires SFstd::derived_from< type_t, AActor >
	{
		const auto World = GetWorldFromContext( WorldContext );
		return GetSingleton( World );
	}
};