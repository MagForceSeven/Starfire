
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Templates/TypeUtilitiesSF.h"

#include "DataStoreUtilities.generated.h"

struct FActorSpawnParameters;

class ADataStoreActor;

// Collection of utilities for working with Data Store Actors and their visualizer actors
UCLASS( )
class STARFIREDATAACTORS_API UDataStoreUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Create an association between a Data Store Actor and an actor to act as its visualizer
	UFUNCTION( BlueprintCallable, Category = "Data Store" )
	static bool AssociateVisualizer( ADataStoreActor *DataStore, AActor *Visualizer );

	// Break the association between a Data Store Actor and the actor acting as its visualizer
	UFUNCTION( BlueprintCallable, Category = "Data Store" )
	static void DisassociateVisualizer( AActor *Actor );

	// Retrieve the Actor currently being used to visualize a Data Actor
	static AActor* GetVisualizer( const ADataStoreActor *DataStore );
	// Retrieve the Data Actor being visualized by an Actor
	static ADataStoreActor* GetDataStoreActor( const AActor *Actor );

	// Retrieve the Actor (as a specific subtype) current being used to visualize a Data Actor
	template < SFstd::derived_from< AActor > type_t >
	static type_t* GetVisualizerAs( const ADataStoreActor *DataStore );

	// Retrieve the Data Actor (as a specific subtype) being visualized by an Actor
	template < SFstd::derived_from< ADataStoreActor > type_t >
	static type_t* GetDataStoreActorAs( const AActor *Actor );

	// Create a visualizer and associate it with a Data Actor. Default to the type of visualizer specified by the Data Actor's type, unless VisualizerOverride is specified
	static AActor* SpawnVisualizer( ADataStoreActor *DataStore, const FTransform &Transform, FActorSpawnParameters SpawnParameters = { }, const TSubclassOf< AActor > &VisualizerOverride = nullptr );
	
	// Create a visualizer and associate it with a Data Actor. Default to the type of visualizer specified by the Data Actor's type, unless VisualizerOverride is specified
	template < SFstd::derived_from< AActor > type_t >
	static type_t* SpawnVisualizerAs( ADataStoreActor *DataStore, const FTransform &Transform, FActorSpawnParameters SpawnParameters = { }, const TSubclassOf< type_t > &VisualizerOverride = nullptr );
};

#if CPP
#define DATA_STORE_UTILITIES_HPP
#include "../Private/DataStoreUtilities.hpp"
#undef DATA_STORE_UTILITIES_HPP
#endif