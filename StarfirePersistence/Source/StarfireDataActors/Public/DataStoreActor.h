
#pragma once

#include "GameFramework/Actor.h"

#include "DataStoreActor.generated.h"

class UPersistenceComponent;

// Base class for data store actors which can act as the back-end source of persistent gameplay data
UCLASS( NotPlaceable, Abstract, Config = "Game" )
class STARFIREDATAACTORS_API ADataStoreActor : public AActor
{
	GENERATED_BODY( )
public:
	ADataStoreActor( );

	// Actor API
	void BeginPlay( ) override;
	void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

#if WITH_EDITOR
	// Specify a custom label to disable in the level outliner
	UFUNCTION( BlueprintNativeEvent )
	FString GetCustomActorLabel( ) const;
	virtual FString GetCustomActorLabel_Implementation( ) const { return FString( ); }

	// Specify a custom directory to organize in the level outliner
	// (will always be a subdirectory of "Data Store"
	UFUNCTION( BlueprintNativeEvent )
	FString GetCustomOutlinerFolder( ) const;
	virtual FString GetCustomOutlinerFolder_Implementation( ) const { return FString( ); }
#endif

	// Retrieve the type of Actor that should be used to visually represent this data actor
 	TSoftClassPtr< AActor > GetVisualizerClass( void ) const { return VisualizerClass; }

#if WITH_EDITOR
	// UObject API
	EDataValidationResult IsDataValid( FDataValidationContext &Context ) const override;
#endif

protected:
	// The type of Actor that should be used to visually represent this data actor
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Config, meta = (OnlyPlaceable, AllowAbstract = false) )
	TSoftClassPtr< AActor > VisualizerClass;

private:
	friend class UDataStoreUtilities;

	// Component to opt these actors in to being persistent
	UPROPERTY( VisibleInstanceOnly )
	TObjectPtr< UPersistenceComponent > PersistenceComponent;

	// The Actor currently being used to represent the data actor
	UPROPERTY( VisibleInstanceOnly )
	TObjectPtr< AActor > Visualizer;
};