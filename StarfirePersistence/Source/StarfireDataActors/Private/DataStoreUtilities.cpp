
#include "DataStoreUtilities.h"

#include "DataStoreActor.h"
#include "DataStoreVisualizer.h"
#include "Module/StarfireDataActors.h"
#include "VisualizerValidator.h"

// Engine
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreUtilities)

bool UDataStoreUtilities::AssociateVisualizer( ADataStoreActor *DataStore, AActor *Visualizer )
{
	if (!ensureAlways( DataStore != nullptr ))
		return false;
	if (!ensureAlways( Visualizer != nullptr ))
		return false;

	if (!ensureAlwaysMsgf( DataStore->Visualizer == nullptr, TEXT( "Failed to associate visualizer. Data Store Actor already associated with a Visual Actor." ) ))
		return false;

	if (Visualizer->IsA< ADataStoreActor >( ))
	{
		UE_LOG( LogStarfireDataActors, Warning, TEXT( "Attempted to associate '%s' as a visualizer but the actor is a data store actor type." ), *Visualizer->GetName( ) );
		return false;
	}

	auto Component = Visualizer->GetComponentByClass< UDataStoreVisualizer >( );
	if (Component != nullptr)
	{
		if (!ensureAlwaysMsgf( Component->DataStoreActor == nullptr, TEXT( "Failed to associate visualizer. Visualizer already associated with Data Store Actor" ) ))
			return false;
	}
	else
	{
		Component = NewObject< UDataStoreVisualizer >( Visualizer );
		
		Visualizer->AddOwnedComponent( Component );
		Component->RegisterComponent( );
	}
	
	const auto DataActorVisType = DataStore->GetVisualizerClass( );
	if (!DataActorVisType.IsNull( ) && UVisualizerValidator::SoftIsChildOf( Visualizer->GetClass( ), DataActorVisType ))
	{
		UE_LOG( LogStarfireDataActors, Error, TEXT( "Unable to associate actor of type '%s' as a visualizer for Data Store Actor of type '%s'. Data Store Actor type expects visualizers of type '%s'." ),
			*Visualizer->GetClass( )->GetName( ), *DataStore->GetClass( )->GetName( ), *DataActorVisType.GetAssetName( ) );
		return false;
	}

	Component->DataStoreActor = DataStore;
	DataStore->Visualizer = Visualizer;

	return true;
}

void UDataStoreUtilities::DisassociateVisualizer( AActor *Actor )
{
	if (Actor == nullptr)
		return;
	
	if (const auto DataStore = Cast< ADataStoreActor >( Actor ))
	{
		if (DataStore->Visualizer == nullptr)
			return;

		const auto Component = DataStore->Visualizer->GetComponentByClass< UDataStoreVisualizer >( );
		if (ensureAlwaysMsgf( Component != nullptr, TEXT( "Actor referenced as a Visualizer does not have a DataStoreVisualizer component" ) ))
			Component->DataStoreActor = nullptr;

		Component->DataStoreActor = nullptr;
		DataStore->Visualizer = nullptr;
	}
	else if (const auto Component = Actor->GetComponentByClass< UDataStoreVisualizer >( ))
	{
		if (Component->DataStoreActor == nullptr)
			return;

		Component->DataStoreActor->Visualizer = nullptr;
		Component->DataStoreActor = nullptr;
	}
	else
	{
		UE_LOG( LogStarfireDataActors, Log, TEXT( "Attempted to disassociate '%s' as a visualizer but the actor is not a data store actor or a visualizer." ), *Actor->GetName( ) );
	}
}

AActor* UDataStoreUtilities::GetVisualizer( const ADataStoreActor *DataStore )
{
	if (!ensureAlways( DataStore != nullptr ))
		return nullptr;

	return DataStore->Visualizer;
}

ADataStoreActor* UDataStoreUtilities::GetDataStoreActor( const AActor *Actor )
{
	if (!ensureAlways( Actor != nullptr))
		return nullptr;
	
	const auto Component = Actor->GetComponentByClass< UDataStoreVisualizer >( );
	if (Component == nullptr)
		return nullptr;

	return Component->DataStoreActor;
}

AActor* UDataStoreUtilities::SpawnVisualizer( ADataStoreActor *DataStore, const FTransform &Transform, FActorSpawnParameters SpawnParameters, const TSubclassOf< AActor > &VisualizerOverride )
{
	if (!ensureAlways( DataStore != nullptr ))
		return nullptr;

	const auto DataStoreVisType = DataStore->GetVisualizerClass( );
	if (DataStoreVisType.IsNull( ) && (VisualizerOverride == nullptr))
	{
		UE_LOG( LogStarfireDataActors, Error, TEXT( "Unable to spawn data store visualizer. No override supplied and '%s' class does not specify one." ), *DataStore->GetClass( )->GetName( ) );
		return nullptr;
	}

	auto VisualizerClass = VisualizerOverride.Get( );
	if (VisualizerClass == nullptr)
	{
		const TArray< FSoftObjectPath > Paths = { DataStoreVisType.ToSoftObjectPath( ) };
		const auto StreamHandle = UAssetManager::Get( ).GetStreamableManager(  ).RequestAsyncLoad( Paths );
		if (!StreamHandle.IsValid( ))
		{
			UE_LOG( LogStarfireDataActors, Error, TEXT( "Unable to start streaming visualizer class '%s'" ), *DataStoreVisType.GetAssetName( ) );
			return nullptr;
		}

		StreamHandle->WaitUntilComplete( );

		VisualizerClass = DataStoreVisType.Get( );
		if (VisualizerClass == nullptr)
		{
			UE_LOG( LogStarfireDataActors, Error, TEXT( "Failed to load visualizer class '%s'" ), *DataStoreVisType.GetAssetName( ) );
			return nullptr;
		}
	}
	else
	{
		if (!DataStoreVisType.IsNull( ) && !UVisualizerValidator::SoftIsChildOf( VisualizerOverride.Get( ), DataStoreVisType ))
		{
			UE_LOG( LogStarfireDataActors, Error, TEXT( "Unable to spawn visualizer of type '%s'. Visualizers for '%s' data store actors must be a child of '%s'." ),
				*VisualizerOverride->GetName( ), *DataStore->GetClass( )->GetName( ), *DataStoreVisType.GetAssetName( ) );
			return nullptr;
		}

		if ((SpawnParameters.Template != nullptr) && (SpawnParameters.Template->GetClass( ) != VisualizerOverride))
		{
			UE_LOG( LogStarfireDataActors, Error, TEXT( "Unable to spawn visualizer of type '%s'. Spawn Parameter Template Actor of a different type (%s) was specified." ),
				*VisualizerOverride->GetName( ), *SpawnParameters.Template->GetClass( )->GetName( ) );
			return nullptr;
		}
	}

	const auto OriginalDeferred = SpawnParameters.bDeferConstruction;
	SpawnParameters.bDeferConstruction = true;

	if (SpawnParameters.SpawnCollisionHandlingOverride == ESpawnActorCollisionHandlingMethod::Undefined)
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const auto World = DataStore->GetWorld( );
	const auto Visualizer = World->SpawnActor< AActor >( VisualizerClass, Transform, SpawnParameters );

	AssociateVisualizer( DataStore, Visualizer );

	if (!OriginalDeferred)
		Visualizer->FinishSpawning( Transform );

	return Visualizer;
}