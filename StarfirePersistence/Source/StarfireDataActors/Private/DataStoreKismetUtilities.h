
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "DataStoreKismetUtilities.generated.h"

enum class ExecEnum_Validity : uint8;

struct FActorSpawnParameters;

class ADataStoreActor;

// Modes that SpawnActor can use the supplied name when it is not None.
// Blueprint accessible version of FActorSpawnParameters.ESpawnActorNameMode
UENUM( )
enum class EDSActorSpawnNameMode : uint8
{
	// Fatal if unavailable, application will assert
	Required_Fatal,

	// Report an error return null if unavailable
	Required_ErrorAndReturnNull,

	// Return null if unavailable
	Required_ReturnNull,

	// If the supplied Name is already in use, generates an unused one using the supplied version as a base
	Requested
};

// A blueprint compatible version of FActorSpawnParameters
USTRUCT( BlueprintType )
struct STARFIREDATAACTORS_API FDSActorSpawnParameters
{
	GENERATED_BODY( )

	// A name to assign as the Name of the Actor being spawned. If no value is specified, the name of the spawned Actor will be automatically generated using the form [Class]_[Number].
	UPROPERTY( BlueprintReadWrite )
	FName Name;

	// An Actor to use as a template when spawning the new Actor. The spawned Actor will be initialized using the property values of the template Actor. If left NULL the class default object (CDO) will be used to initialize the spawned Actor.
	UPROPERTY( BlueprintReadWrite )
	TObjectPtr< AActor > Template = nullptr;

	// The Actor that spawned this Actor. (Can be left as NULL).
	UPROPERTY( BlueprintReadWrite )
	TObjectPtr< AActor > Owner = nullptr;

	// The APawn that is responsible for damage done by the spawned Actor. (Can be left as NULL).
	UPROPERTY( BlueprintReadWrite )
	TObjectPtr< APawn >	Instigator = nullptr;

	// The ULevel to spawn the Actor in, i.e. the Outer of the Actor. If left as NULL the Outer of the Owner is used. If the Owner is NULL the persistent level is used.
	UPROPERTY( BlueprintReadWrite, meta = (HidePinAssetPicker = true) )
	TObjectPtr< ULevel > OverrideLevel = nullptr;

	// Method for resolving collisions at the spawn point. Undefined means no override, use the actor's setting.
	UPROPERTY( BlueprintReadWrite )
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;

	// Determines whether to multiply or override root component with provided spawn transform
	UPROPERTY( BlueprintReadWrite )
	ESpawnActorScaleMethod TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	// Determines whether spawning will not fail if certain conditions are not met. If true, spawning will not fail because the class being spawned is `bStatic=true` or because the class of the template Actor is not the same as the class of the Actor being spawned.
	UPROPERTY( BlueprintReadWrite )
	bool bNoFail = false;

	// Determines whether the construction script will be run. If true, the construction script will not be run on the spawned Actor. Only applicable if the Actor is being spawned from a Blueprint.
	UPROPERTY( BlueprintReadWrite )
	bool bDeferConstruction = false;
	
	// Determines whether or not the actor may be spawned when running a construction script. If true spawning will fail if a construction script is being run.
	UPROPERTY( BlueprintReadWrite )
	bool bAllowDuringConstructionScript = false;

	// In which way should SpawnActor should treat the supplied Name if not none.
	UPROPERTY( BlueprintReadWrite )
	EDSActorSpawnNameMode NameMode = EDSActorSpawnNameMode::Requested;

	// Conversion from the blueprint compatible version to the Engine version of this structure
	FActorSpawnParameters ToActorSpawnParameters( ) const;
};

// Utilities only used by blueprint or used for the implementation of custom blueprint nodes
UCLASS( )
class UDataStoreKismetUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Utility for the custom node to retrieve a visualizer Actor for a Data Actor
	// TODO: Custom Node
	UFUNCTION( BlueprintCallable, Category = "Data Store", meta = (DeterminesOutputType = "ActorType", ExpandEnumAsExecs = Exec) )
	static AActor* GetVisualizerAs( const ADataStoreActor *DataStore, TSubclassOf< AActor > ActorType, ExecEnum_Validity &Exec );
	
	// Retrieve the Data Actor being visualized by an Actor
	UFUNCTION( BlueprintCallable, Category = "Data Store", meta = (DeterminesOUtputType = "ActorType", ExpandEnumAsExecs = Exec) )
	static ADataStoreActor* GetDataStoreActorAs( const AActor *Actor, TSubclassOf< ADataStoreActor > ActorType, ExecEnum_Validity &Exec );

	// Utility for the custom node to spawn & associate a visualizer for a Data Actor
	// TODO: Custom Node
	UFUNCTION( BlueprintCallable, Category = "Data Store", meta = (ExpandEnumAsExecs = Exec) )
	static AActor* SpawnVisualizer( ADataStoreActor *DataStore, const FTransform &Transform, FDSActorSpawnParameters SpawnParameters, TSubclassOf< AActor > ActorType, ExecEnum_Validity &Exec );

};