
#include "ActorCollectionUtilities.h"

#include "ActorCollectionBase.h"
#include "ActorCollection.h"
#include "ActorCollectionSingleton.h"
#include "FilteredActorCollection.h"

// Engine
#include "EngineUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorCollectionUtilities)

TArray< AActorCollectionBase* > UActorCollectionUtilities::GetCollections( const UObject *WorldContext, TSubclassOf< AActorCollectionBase > CollectionType )
{
	static const TArray Bases = { AActorCollectionBase::StaticClass( ), AActorCollection::StaticClass( ), AFilteredActorCollection::StaticClass( ), AActorCollectionSingleton::StaticClass( ) };

	if ((CollectionType == nullptr) || Bases.Contains( CollectionType ))
		return { };

	TArray< AActorCollectionBase* > Results;

	if (const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull ))
	{
		for (const auto Collection : TActorRange( World, CollectionType ))
		{
			if (IsValid( Collection ))
				Results.Add( Collection );
		}
	}

	return Results;
}

AActorCollectionSingleton* UActorCollectionUtilities::GetOrCreateSingletonCollection( const UObject *WorldContext, TSubclassOf< AActorCollectionSingleton > CollectionType )
{
	if ((CollectionType == nullptr) || (CollectionType == AActorCollectionSingleton::StaticClass( )))
		return nullptr;

	if (const auto Collection = GetSingletonCollection( WorldContext, CollectionType ))
		return Collection;

	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bNoFail = true;
			
	const auto NewCollection = World->SpawnActor< AActorCollectionSingleton >( CollectionType, Params);
	ensureAlways( NewCollection != nullptr );

	return NewCollection;
}

AActorCollectionSingleton* UActorCollectionUtilities::GetSingletonCollection( const UObject *WorldContext, TSubclassOf< AActorCollectionSingleton > CollectionType )
{
	if ((CollectionType == nullptr) || (CollectionType == AActorCollectionSingleton::StaticClass( )))
		return nullptr;

	const auto Collections = GetCollections( WorldContext, CollectionType );

	if (Collections.IsEmpty( ))
		return nullptr;

	ensureAlways( Collections.Num( ) == 1 );

	return CastChecked< AActorCollectionSingleton >( Collections[ 0 ] );
}

AFilteredActorCollection* UActorCollectionUtilities::CreateFilteredCollectionFromSingleton( const UObject *WorldContext, TSubclassOf< AFilteredActorCollection > FilterType, TSubclassOf< AActorCollectionSingleton > SingletonType )
{
	if ((FilterType == nullptr) || (FilterType == AFilteredActorCollection::StaticClass( )))
		return nullptr;

	if ((SingletonType == nullptr) || (SingletonType == AActorCollectionSingleton::StaticClass( )))
		return nullptr;

	const auto Singleton = GetOrCreateSingletonCollection( WorldContext, SingletonType );

	return CreateFilteredCollection( FilterType, Singleton );
}

AFilteredActorCollection* UActorCollectionUtilities::CreateFilteredCollection( TSubclassOf< AFilteredActorCollection > FilterType, AActorCollectionBase *Collection )
{
	if ((FilterType == nullptr) || (FilterType == AFilteredActorCollection::StaticClass( )))
		return nullptr;

	if (!ensureAlways( IsValid(Collection) ))
		return nullptr;

	const auto World = Collection->GetWorld( );
	if (!ensureAlways( IsValid( World ) ))
		return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.bNoFail = true;
	Params.bDeferConstruction = true;
			
	const auto NewCollection = World->SpawnActor< AFilteredActorCollection >( FilterType, Params);
	ensureAlways( NewCollection != nullptr );

	NewCollection->InitSourceCollection( Collection );

	NewCollection->FinishSpawning( FTransform::Identity );

	return NewCollection;
}