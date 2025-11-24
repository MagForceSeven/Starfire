
#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "Templates/TypeUtilitiesSF.h"

#include "PersistentDataStore.generated.h"

class ADataStoreActor;
class ADataStoreSingleton;
class UDataStoreSettings;
struct FStreamableHandle;

// Subsystem that collects all Data Store Actors and provides a single point of access for lookups
// and singleton spawning
UCLASS( )
class STARFIREDATAACTORS_API UPersistentDataStore : public UWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Native accessor for getting at the subsystem
	[[nodiscard]] static UPersistentDataStore* GetSubsystem( const UObject* WorldContext );

	// Spawn a singleton data actor (maybe spawning a subtype based on the settings)
	template < SFstd::derived_from< ADataStoreSingleton > type_t >
	type_t* SpawnSingleton( bool bDeferredSpawning = false );

	// Spawn a singleton data actor (maybe spawning a subtype based on the settings)
	template < SFstd::derived_from< ADataStoreSingleton > type_t >
	static type_t* SpawnSingleton( const UObject* WorldContext, bool bDeferredSpawning = false );

	// Retrieve a data store actor with a specific ID
	template < SFstd::derived_from< ADataStoreActor > type_t >
	[[nodiscard]] type_t* GetDataStoreActor( const FGuid &ID ) const;

	// Retrieve a singleton data actor based on a specific type
	template < SFstd::derived_from< ADataStoreSingleton > type_t >
	[[nodiscard]] type_t* GetSingleton( void ) const;
	
	// Retrieve a singleton data actor based on a specific type
	template < SFstd::derived_from< ADataStoreSingleton > type_t >
	[[nodiscard]] static type_t* GetSingleton( const UObject* WorldContext );

	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;

private:
	friend class ADataStoreActor;
	friend class ADataStoreSingleton;

	// Add a new actor to the lookups
	void AddDataStoreActor( ADataStoreActor *Actor, const FGuid &Guid );
	// Remove an actor from the lookups
	void RemoveDataStoreActor( ADataStoreActor *Actor, const FGuid &Guid );

	// Add a new actor to the lookups
	void AddSingleton( ADataStoreSingleton *Actor );
	// Remove an actor from the lookups
	void RemoveSingleton( ADataStoreSingleton *Actor );

	// The collection of active Data Store Actors and their associated GUIDS
	UPROPERTY( VisibleInstanceOnly )
	TMap< FGuid, TObjectPtr< ADataStoreActor > > Actors;

	// The collection of active singleton data store actors for given classes
	UPROPERTY( VisibleInstanceOnly )
	TMap< TObjectPtr< const UClass >, TObjectPtr< ADataStoreSingleton > > Singletons;

	// The set of singleton types that are shared base classes between multiple singletons
	// and therefore invalid inputs for GetSingletonDataActor
	UPROPERTY( VisibleInstanceOnly )
	TSet< TObjectPtr< const UClass >> InvalidSingletonTypes;

	// Cached reference to the settings
	UPROPERTY( VisibleInstanceOnly )
	TObjectPtr< const UDataStoreSettings > Settings;

	// Spawn a singleton data actor (maybe spawning a subtype based on the settings)
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = SingletonType, AdvancedDisplay = "bDeferredSpawning") )
	ADataStoreSingleton* SpawnSingleton( TSubclassOf< ADataStoreSingleton > SingletonType, bool bDeferredSpawning = false );

	// Spawn a singleton data actor (maybe spawning a subtype based on the settings)
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = SingletonType, WorldContext = "WorldContext", AdvancedDisplay = "bDeferredSpawning") )
	static ADataStoreSingleton* SpawnDataStoreSingleton( const UObject *WorldContext, TSubclassOf< ADataStoreSingleton > SingletonType, bool bDeferredSpawning = false );

	// Retrieve a data store actor with a specific ID
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = ActorType) )
	ADataStoreActor* GetDataStoreActor( const TSubclassOf< ADataStoreActor > &ActorType, const FGuid &ID ) const;

	// Retrieve a singleton data actor based on a specific type
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = SingletonType) )
	ADataStoreSingleton* GetSingleton( TSubclassOf< ADataStoreSingleton > SingletonType ) const;

	// Retrieve a singleton data actor based on a specific type
	UFUNCTION( BlueprintCallable, meta = (DeterminesOutputType = SingletonType, WorldContext = "WorldContext") )
	static ADataStoreSingleton* GetDataStoreSingleton( const UObject *WorldContext, TSubclassOf< ADataStoreSingleton > SingletonType );

	// Resource loading of the singleton types from the settings
	TSharedPtr< FStreamableHandle > SingletonTypesLoading;
};

#if CPP
#define PERSISTENT_DATA_STORE_HPP
#include "../Private/PersistentDataStore.hpp"
#undef PERSISTENT_DATA_STORE_HPP
#endif