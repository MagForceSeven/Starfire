
#include "DataStoreKismetUtilities.h"

#include "DataStoreActor.h"
#include "DataStoreUtilities.h"

#include "Kismet/BlueprintUtilitiesSF.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreKismetUtilities)

AActor* UDataStoreKismetUtilities::GetVisualizerAs( const ADataStoreActor *DataStore, TSubclassOf< AActor > ActorType, ExecEnum_Validity &Exec )
{
	Exec = ExecEnum_Validity::Invalid;

	if (ActorType == nullptr)
		return nullptr;
	
	const auto Visualizer = UDataStoreUtilities::GetVisualizer( DataStore );
	if (Visualizer == nullptr)
		return nullptr;

	if (!Visualizer->IsA( ActorType ))
		return nullptr;

	Exec = ExecEnum_Validity::Valid;
	return Visualizer;
}

ADataStoreActor* UDataStoreKismetUtilities::GetDataStoreActorAs( const AActor *Actor, TSubclassOf< ADataStoreActor > ActorType, ExecEnum_Validity &Exec )
{
	Exec = ExecEnum_Validity::Invalid;

	if (ActorType == nullptr)
		return nullptr;

	const auto DataStore = UDataStoreUtilities::GetDataStoreActor( Actor );
	if (DataStore == nullptr)
		return nullptr;

	if (!DataStore->IsA( ActorType ))
		return nullptr;

	return DataStore;
}

AActor* UDataStoreKismetUtilities::SpawnVisualizer( ADataStoreActor *DataStore, const FTransform &Transform, FDSActorSpawnParameters SpawnParameters, TSubclassOf< AActor > ActorType, ExecEnum_Validity &Exec )
{
	const auto Visualizer = UDataStoreUtilities::SpawnVisualizer( DataStore, Transform, SpawnParameters.ToActorSpawnParameters( ), ActorType );

	Exec = (Visualizer != nullptr) ? ExecEnum_Validity::Valid : ExecEnum_Validity::Invalid;
	
	return Visualizer;
}

FActorSpawnParameters FDSActorSpawnParameters::ToActorSpawnParameters( ) const
{
#if WITH_EDITOR
	// Collection of asserts to make sure we can do static casts from our enum to the engine enum
	// ReSharper disable CppCStyleCast
	static_assert( (int)EDSActorSpawnNameMode::Requested == (int)FActorSpawnParameters::ESpawnActorNameMode::Requested );
	static_assert( (int)EDSActorSpawnNameMode::Required_ReturnNull == (int)FActorSpawnParameters::ESpawnActorNameMode::Required_ReturnNull );
	static_assert( (int)EDSActorSpawnNameMode::Required_ErrorAndReturnNull == (int)FActorSpawnParameters::ESpawnActorNameMode::Required_ErrorAndReturnNull );
	static_assert( (int)EDSActorSpawnNameMode::Required_Fatal == (int)FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal );
	// ReSharper restore CppCStyleCast

	// A do-nothing switch that exercises the known enum elements but with a warning as error enabled to signal any new elements this plugin is missing
#pragma warning( push )
#pragma warning( error : 4062 ) // Enable the missing-enumeration-value-switch-case warning as an error to keep our enumeration properly mirrored
	switch ( FActorSpawnParameters::ESpawnActorNameMode() )
	{
		case FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal:
			break;
		case FActorSpawnParameters::ESpawnActorNameMode::Required_ErrorAndReturnNull:
			break;
		case FActorSpawnParameters::ESpawnActorNameMode::Required_ReturnNull:
			break;
		case FActorSpawnParameters::ESpawnActorNameMode::Requested:
			break;
	}
#pragma warning( pop )
	
#endif
	
	FActorSpawnParameters SpawnParameters;

	SpawnParameters.Name							= Name;
	SpawnParameters.Template						= Template;
	SpawnParameters.Owner							= Owner;
	SpawnParameters.Instigator						= Instigator;
	SpawnParameters.OverrideLevel					= OverrideLevel;
	SpawnParameters.SpawnCollisionHandlingOverride	= SpawnCollisionHandlingOverride;
	SpawnParameters.TransformScaleMethod			= TransformScaleMethod;
	SpawnParameters.bNoFail							= bNoFail;
	SpawnParameters.bDeferConstruction				= bDeferConstruction;
	SpawnParameters.bAllowDuringConstructionScript	= bAllowDuringConstructionScript;
	SpawnParameters.NameMode						= static_cast< FActorSpawnParameters::ESpawnActorNameMode >( NameMode );

	return SpawnParameters;
}