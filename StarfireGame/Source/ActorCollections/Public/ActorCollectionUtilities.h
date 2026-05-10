
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/TypeUtilitiesSF.h"

#include "ActorCollectionUtilities.generated.h"

class AActorCollectionBase;
class AActorCollection;
class AActorCollectionSingleton;
class AFilteredActorCollection;

// Concept to accept any collection type that is not one of the abstract bases provided by this plugin
template < class type_t >
concept CCollectionType = SFstd::derived_from< type_t, AActorCollectionBase > &&
							!std::is_same_v< type_t, AActorCollectionBase > &&
							!std::is_same_v< type_t, AActorCollection > &&
							!std::is_same_v< type_t, AFilteredActorCollection > &&
							!std::is_same_v< type_t, AActorCollectionSingleton >;

// Concept to accept any collection type that is derived from the singleton base class, but is not _the_ singleton base class
template < class type_t >
concept CSingletonCollectionType = SFstd::derived_from< type_t, AActorCollectionSingleton > && !std::is_same_v< type_t, AActorCollectionSingleton >;

// Concept to accept any collection type that is derived from the filtered collection base class, but is not _the_ filtered collection base class
template < class type_t >
concept CFilteredCollectionType = SFstd::derived_from< type_t, AFilteredActorCollection > && !std::is_same_v< type_t, AFilteredActorCollection >;

// Manager that tracks all actor collections, provides factories for singleton and filtered collections and
// provides lookup based on type
UCLASS( )
class ACTORCOLLECTIONS_API UActorCollectionUtilities final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Lookup all the collections of a specific type
	template < CCollectionType type_t >
	[[nodiscard]] static TArray< type_t* > GetCollections( const UObject *WorldContext );

	// Get a singleton collection or create it if one of that type does not exist
	template < CSingletonCollectionType type_t >
	[[nodiscard]] static type_t* GetOrCreateSingletonCollection( const UObject *WorldContext );

	// Get a singleton collection based on the desired type
	template < CSingletonCollectionType type_t >
	[[nodiscard]] static type_t* GetSingletonCollection( const UObject *WorldContext );

	// Create a new filtered collection based on a singleton collection (creating the singleton collection if necessary)
	template < CFilteredCollectionType filter_t, CSingletonCollectionType singleton_t >
	[[nodiscard]] static filter_t* CreateFilteredCollection( const UObject *WorldContext );

	// Create a new filtered collection based on a singleton collection (creating the singleton collection if necessary)
	template < CFilteredCollectionType filter_t >
	[[nodiscard]] static filter_t* CreateFilteredCollection( const UObject *WorldContext, const TSubclassOf< AActorCollectionSingleton > &SingletonType );

	// Create a new filtered collection based on an existing actor collection
	template < CFilteredCollectionType filter_t >
	[[nodiscard]] static filter_t* CreateFilteredCollection( AActorCollectionBase *Collection );

	// *****************************************************************************************************************
	// Blueprint Compatible API

	// Get a singleton collection or create it if one of that type does not exist
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "CollectionType") )
	[[nodiscard]] static AActorCollectionSingleton* GetOrCreateSingletonCollection( const UObject *WorldContext, TSubclassOf< AActorCollectionSingleton > CollectionType );
	
	// Get a singleton collection based on the desired type
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "CollectionType") )
	[[nodiscard]] static AActorCollectionSingleton* GetSingletonCollection( const UObject *WorldContext, TSubclassOf< AActorCollectionSingleton > CollectionType );

	// Create a new filtered collection based on a singleton collection (creating the singleton collection if necessary)
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "FilterType") )
	[[nodiscard]] static AFilteredActorCollection* CreateFilteredCollectionFromSingleton( const UObject *WorldContext, TSubclassOf< AFilteredActorCollection > FilterType, TSubclassOf< AActorCollectionSingleton > SingletonType );

	// Create a new filtered collection based on an existing actor collection
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = "FilterType") )
	[[nodiscard]] static AFilteredActorCollection* CreateFilteredCollection( TSubclassOf< AFilteredActorCollection > FilterType, AActorCollectionBase *Collection );

private:
	// Lookup all the collections of a specific type
	[[nodiscard]] static TArray< AActorCollectionBase* > GetCollections( const UObject *WorldContext, TSubclassOf< AActorCollectionBase > CollectionType );
};

#if CPP
#define ACTOR_COLLECTION_UTILITIES_HPP
#include "../Private/ActorCollectionUtilities.hpp"
#undef ACTOR_COLLECTION_UTILITIES_HPP
#endif