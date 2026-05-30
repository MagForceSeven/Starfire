
#include "PersistentDataStore.h"

#include "DataStoreActor.h"
#include "DataStoreSettings.h"
#include "DataStoreSingleton.h"
#include "Module/StarfireDataActors.h"

// Engine
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PersistentDataStore)

TSharedPtr< FStreamableHandle > UPersistentDataStore::OverrideTypesLoading;
TMap< TSubclassOf< ADataStoreActor >, TSubclassOf< ADataStoreActor > > UPersistentDataStore::ClassOverrides;

[[nodiscard]] static UClass* GetFirstNativeParent( UClass* Type )
{
	check( Type != nullptr );

	while (Type != nullptr)
	{
		if (Type->IsNative( ))
			return Type;
		
		Type = Type->GetSuperClass();
	}

	UE_ASSUME( false );
}

void UPersistentDataStore::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	Settings = GetDefault< UDataStoreSettings >( );

	if (!Settings->BlueprintOverrides.IsEmpty( ) && !OverrideTypesLoading.IsValid( ))
	{
		TArray< FSoftObjectPath > TypePaths;
		Algo::Transform( Settings->BlueprintOverrides, TypePaths,
			[ ]( const TSoftClassPtr< ADataStoreActor > &Type ) -> FSoftObjectPath { return Type.ToSoftObjectPath( ); } );
		TypePaths.RemoveAll( [ ]( const FSoftObjectPath& Path ) -> bool { return Path.IsNull( ); } );

		const auto OnLoadComplete = [ WeakThis = TWeakObjectPtr(this) ]( ) -> void
		{
			static const auto DataActorClass = ADataStoreActor::StaticClass( );
			static const auto SingletonActorClass = ADataStoreSingleton::StaticClass( );
		
			if (!WeakThis.IsValid( ))
				return;
		
			for (const auto &Type : WeakThis->Settings->BlueprintOverrides)
			{
				if (!ensureAlwaysMsgf( !Type->IsNative( ), TEXT( "Native type '%s' found in Data Store Settings - Blueprint Overrides." ), *Type->GetName( ) ))
					continue;

				const auto NativeParent = GetFirstNativeParent( Type.Get( ) );
				if (NativeParent == DataActorClass)
					continue;
				if (NativeParent == SingletonActorClass)
					continue;

				ensureAlwaysMsgf( ClassOverrides.Find( NativeParent ) == nullptr, TEXT( "Multiple Data Store Actor overrides found for class '%s'." ), *NativeParent->GetName( ) );
				ClassOverrides.Add( NativeParent, Type.Get( ) );
			}
		};

		OverrideTypesLoading = UAssetManager::Get( ).GetStreamableManager( ).RequestAsyncLoad( TypePaths, OnLoadComplete );
	}
}

bool UPersistentDataStore::DoesSupportWorldType( const EWorldType::Type WorldType ) const
{
	return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE);
}

void UPersistentDataStore::AddDataStoreActor( ADataStoreActor *Actor, const FGuid &Guid )
{
	Actors.Add( Guid, Actor );
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
			TObjectPtr< ADataStoreSingleton > Value;
			if (Singletons.RemoveAndCopyValue( InstanceType, Value ))
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

	if (OverrideTypesLoading.IsValid( ) && !OverrideTypesLoading->HasLoadCompleted( ))
		OverrideTypesLoading->WaitUntilComplete( );

	if (SingletonType->IsNative( ))
	{
		if (const auto Found = ClassOverrides.Find( SingletonType ))
			SingletonType = *Found;
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

TSubclassOf<ADataStoreActor> UPersistentDataStore::GetOverrideClassFor( const TSubclassOf<ADataStoreActor> &BaseType )
{
	if (OverrideTypesLoading.IsValid( ) && !OverrideTypesLoading->HasLoadCompleted( ))
		OverrideTypesLoading->WaitUntilComplete( );
	
	if (const auto Found = ClassOverrides.Find( BaseType ))
		return *Found;

	return BaseType;
}
