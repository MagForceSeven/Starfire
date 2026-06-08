
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/TypeUtilitiesSF.h"

#include "ActorVMUtilities.generated.h"

class UActorVMBase;

// Concept to limit template classes to types derived from ActorVMBase
template < class type_t >
concept CActorVMType = SFstd::derived_from< type_t, UActorVMBase >;

// A collection of utilities for working with Actor View Models
UCLASS( )
class ACTORVIEWMODELS_API UActorVMUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Get an Actor VM for a specific Actor, returning an existing instance of the requested type or creating one
	template < CActorVMType type_t >
	[[nodiscard]] static type_t* FindOrCreateVM( AActor *Actor );

	// Get an Actor VM for the same Actor as another ActorVM, returning an existing instance of the requested type or creating one
	template < CActorVMType type_t >
	[[nodiscard]] static type_t* FindOrCreateVM( const UActorVMBase *ActorVM );

	// Get an Actor VM for the Actor associated with a component, returning an existing instance of the requested type or creating one
	template < CActorVMType type_t >
	[[nodiscard]] static type_t* FindOrCreateVM( const UActorComponent *Component );

	// Get an Actor VM for a specific Actor, returning an existing instance of the requested type or creating one
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "VMType") )
	[[nodiscard]] static UActorVMBase* FindOrCreateVM( AActor *Actor, TSubclassOf< UActorVMBase > VMType );

	// Get an Actor VM for the same Actor as another ActorVM, returning an existing instance of the requested type or creating one
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "VMType") )
	[[nodiscard]] static UActorVMBase* FindOrCreateSiblingVM( const UActorVMBase *ActorVM, TSubclassOf< UActorVMBase > VMType );

	// Get an Actor VM for the Actor associated with a component, returning an existing instance of the requested type or creating one
	[[nodiscard]] static UActorVMBase* FindOrCreateVM( const UActorComponent *Component, TSubclassOf< UActorVMBase > VMType );

private:
	friend class UActorVMResolver;

	// Internal system utility for finding or creating an Actor VM
	[[nodiscard]] static UActorVMBase* FindOrCreateVM( AActor *Actor, const UClass *Class );
};

#if CPP
#define ACTOR_VM_UTILITIES_HPP
#include "../../Private/ActorViewModels/ActorVMUtilities.hpp"
#undef ACTOR_VM_UTILITIES_HPP
#endif