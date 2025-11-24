
#include "PersistentDataStore.h"

#include "DataStoreActor.h"
#include "DataStoreSettings.h"
#include "DataStoreSingleton.h"
#include "Module/StarfireDataActors.h"

// Engine
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PersistentDataStore)

UPersistentDataStore* UPersistentDataStore::GetSubsystem( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	return World->GetSubsystem< UPersistentDataStore >( );
}

void UPersistentDataStore::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	Settings = GetDefault< UDataStoreSettings >( );
	TArray< FSoftObjectPath > TypePaths;
	Algo::Transform( Settings->SingletonTypes, TypePaths,
		[ ]( const TSoftClassPtr< ADataStoreSingleton > &Type ) -> FSoftObjectPath { return Type.ToSoftObjectPath( ); } );

	SingletonTypesLoading = UAssetManager::Get( ).GetStreamableManager( ).RequestAsyncLoad( TypePaths );
}

void UPersistentDataStore::AddDataStoreActor( ADataStoreActor *Actor, const FGuid &Guid )
{
	Actors.Add( Guid, Actor );

#if WITH_EDITOR
	const auto Label = Actor->GetCustomActorLabel( );
	if (!Label.IsEmpty( ))
		Actor->SetActorLabel( Label );

	auto FolderPath = Actor->GetCustomOutlinerFolder( );
	if (FolderPath.IsEmpty( ))
		FolderPath = "Data Store";
	else
		FolderPath = "Data Store/" + FolderPath;

	Actor->SetFolderPath( FName( FolderPath ) );
#endif
}

void UPersistentDataStore::RemoveDataStoreActor( ADataStoreActor *Actor, const FGuid &Guid )
{
	Actors.Remove( Guid );
}

void UPersistentDataStore::AddSingleton( ADataStoreSingleton *Actor )
{
	static const auto SingletonType = ADataStoreSingleton::StaticClass( );

	bool bShouldError = true;
	auto InstanceType = Actor->GetClass( );
	while (InstanceType != SingletonType)
	{
		ensureAlwaysMsgf( !bShouldError || (Singletons.Find( InstanceType ) == nullptr), TEXT("Multiple instances of Singleton Data Actor class %s created."), *InstanceType->GetName( ) );

		if (Singletons.Find( InstanceType ) == nullptr)
		{
			Singletons.Add( InstanceType, Actor );
		}
		else if (InvalidSingletonTypes.Contains( InstanceType ))
		{
			break;
		}
		else
		{
			UE_LOG( LogStarfireDataActors, Log, TEXT( "Multiple singletons types found with shared base class %s." ), *InstanceType->GetName( ) );
			Singletons.Remove( InstanceType );
			InvalidSingletonTypes.Add( InstanceType );
			break;
		}
			
		InstanceType = InstanceType->GetSuperClass( );
		bShouldError = false;
	}
}

void UPersistentDataStore::RemoveSingleton( ADataStoreSingleton *Actor )
{
	static const auto SingletonType = ADataStoreSingleton::StaticClass( );

	auto InstanceType = Actor->GetClass( );
	while (InstanceType != SingletonType)
	{
		if (Singletons.Find( InstanceType ) != nullptr)
		{
			const auto Value = Singletons.FindAndRemoveChecked( InstanceType );
			ensureAlways( Value == Actor );
		}
		else if (InvalidSingletonTypes.Contains( InstanceType ))
		{
			break;
		}
			
		InstanceType = InstanceType->GetSuperClass( );
	}
}


ADataStoreSingleton* UPersistentDataStore::SpawnSingleton( TSubclassOf< ADataStoreSingleton > SingletonType, bool bDeferredSpawning )
{
	if (!ensureAlways( SingletonType != nullptr ))
		return nullptr;
	
	if (SingletonTypesLoading.IsValid( ) && !SingletonTypesLoading->HasLoadCompleted( ))
		SingletonTypesLoading->WaitUntilComplete( );

	if (SingletonType->IsNative( ))
	{
		for (const auto &ClassPtr : Settings->SingletonTypes)
		{
			if (ClassPtr == nullptr)
				continue;

			if (ClassPtr->IsChildOf( SingletonType ))
			{
				SingletonType = ClassPtr.Get( );
				break;
			}
		}
	}

	FActorSpawnParameters Params;
	Params.bDeferConstruction = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const auto Singleton = GetWorld( )->SpawnActor< ADataStoreSingleton >( SingletonType.Get( ), Params );

	if (!bDeferredSpawning)
		Singleton->FinishSpawning( FTransform::Identity );

	return Singleton;
}

ADataStoreSingleton* UPersistentDataStore::SpawnDataStoreSingleton( const UObject *WorldContext, TSubclassOf< ADataStoreSingleton > SingletonType, bool bDeferredSpawning )
{
	const auto Subsystem = GetSubsystem( WorldContext );
	check( Subsystem != nullptr );

	return Subsystem->SpawnSingleton( SingletonType, bDeferredSpawning );
}

ADataStoreActor* UPersistentDataStore::GetDataStoreActor( const TSubclassOf< ADataStoreActor > &ActorType, const FGuid &ID ) const
{
	if (!ensureAlways( ActorType != nullptr ))
		return nullptr;

	const auto Found = Actors.Find( ID );
	if (Found == nullptr)
		return nullptr;

	if (!(*Found)->IsA( ActorType ))
		return nullptr;

	return *Found;
}

ADataStoreSingleton* UPersistentDataStore::GetSingleton( TSubclassOf< ADataStoreSingleton > SingletonType ) const
{
	if (!ensureAlways( SingletonType != nullptr ))
		return nullptr;

	const auto Found = Singletons.Find( SingletonType );
	if (Found == nullptr)
		return nullptr;

	return *Found;
}

ADataStoreSingleton* UPersistentDataStore::GetDataStoreSingleton( const UObject *WorldContext, TSubclassOf< ADataStoreSingleton > SingletonType )
{
	const auto Subsystem = GetSubsystem( WorldContext );
	check( Subsystem != nullptr );

	return Subsystem->GetSingleton( SingletonType );
}