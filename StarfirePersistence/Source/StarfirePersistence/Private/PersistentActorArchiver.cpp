
#include "PersistentActorArchiver.h"

#include "ArchiveUtilities.h"
#include "PeristentArchiveVersion.h"
#include "PersistenceShared.h"

#include "PersistenceManager.h"
#include "PersistenceComponent.h"
#include "PersistentSubsystemInterface.h"
#include "StarfirePersistenceSettings.h"
#include "Module/StarfirePersistence.h"

#include "Templates/ArrayTypeUtilitiesSF.h"

// Engine
#include "GameFramework/GameStateBase.h"

// Core
#include "Logging/StructuredLog.h"

enum class ESectionID : uint32
{
	DestroyedActors = 0,
};

//*********************************************************************
// Persistent Actor Archiver
//*********************************************************************

FPersistentActorArchiver::FPersistentActorArchiver( FArchive &InInnerArchive, bool bLoadIfFindFails ) :
	FObjectAndNameAsStringProxyArchive( InInnerArchive, bLoadIfFindFails )
{
	FArchive::SetIsPersistent( true );
	FArchive::SetWantBinaryPropertySerialization( false );
	ArIsSaveGame = true;
}

// *********************************************************************************************************************
// Persistent Actor Writer Archive
// *********************************************************************************************************************

FPersistentActorWriter::FPersistentActorWriter( FArchive& InInnerArchive ) :
	FPersistentActorArchiver( InInnerArchive, false )
{
	FArchive::SetIsSaving( true );

	Settings = GetDefault< UStarfirePersistenceSettings >( );
}

void FPersistentActorWriter::Archive( const UObject *WorldContext )
{
	check( WorldContext != nullptr );
	check( ReferencedObjectList.IsEmpty( ) );
	
	const UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	check( World != nullptr );

	TArray< TObjectPtr< UObject > > ToArchive;

	// Get all the actors that want to be persisted
	const auto Manager = UPersistenceManager::GetSubsystem( WorldContext );
	Manager->PersistentActors.GenerateValueArray( ArrayDownCast< AActor >( ToArchive ) );
	
	// Find all the subsystems that want to persist
	for (TObjectIterator< USubsystem > It; It; ++It)
	{
		if (It->GetWorld( ) != World)
			continue;

		if (!It->Implements< UPersistentSubsystem >( ))
			continue;

		ToArchive.Push( *It );
	}

	Archive( ToArchive );

	SerializeDestroyedActors( Manager );
}

void FPersistentActorWriter::Archive( const TArray< UObject* > &Objects )
{
	auto Version = EPersistenceVersion::Latest;
	*this << Version;

	check( ReferencedObjectList.IsEmpty( ) );

	// Convert the object list into ObjectRecords
	for (const auto Obj : Objects)
	{
		auto& Entry = ReferencedObjectList.Emplace_GetRef( Obj );
		ObjectToReferenceIndexMap.Add( Entry.Object, ReferencedObjectList.Num( ) - 1 );

		const auto Actor = Cast< AActor >( Obj );
		const auto Component = Actor ? Actor->GetComponentByClass< UPersistenceComponent >( ) : nullptr;

		if (Component != nullptr)
		{
			Entry.PersistentID = Component->GetGuid( );
			Entry.bWasSpawned = Component->WasSpawned( );

			Component->OnPreSerialize.Broadcast( );
		}

		RecursiveCollectObjects( Entry.Object, Component, 0 );
	}

#if !UE_BUILD_SHIPPING
	TMap< UClass*, TArray< UObject* > > BucketMapping;
	for (const auto &E : ReferencedObjectList)
		BucketMapping.FindOrAdd( E.ClassPtr.Get( ) ).Push( E.Object );
#endif

	*this << ReferencedObjectList;

	// Then write all the actual object data
	for (const auto& Entry : ReferencedObjectList)
	{
		const int64 SizeStart = Tell( );

		int64 Size = 0; // Reserve space for size information
		*this << Size;

		Entry.Object->Serialize( *this );

		if (AActor* Actor = Cast< AActor >( Entry.Object ))
		{
			AActor* Owner = Actor->GetOwner( );
			*this << Owner;

			const auto Component = Actor ? Actor->GetComponentByClass< UPersistenceComponent >( ) : nullptr;

			if (Component && Component->bPersistTransform)
			{
				FTransform ActorTransform = Actor->GetActorTransform( );
				*this << ActorTransform;
			}
		}

		// seek back & write the size then come back
		const int64 FinalPos = Tell( );
		Size = FinalPos - SizeStart - sizeof( Size );

		Seek( SizeStart );
		*this << Size;

		Seek( FinalPos );
	}

	// Reset these in case Archive is called repeatedly
	ReferencedObjectList.Empty( );
	ObjectToReferenceIndexMap.Empty( );
}

void FPersistentActorWriter::SerializeDestroyedActors( UPersistenceManager *Manager )
{
	if (Manager->DestroyedActors.IsEmpty( ))
		return;
	
	ESectionID SectionID = ESectionID::DestroyedActors;
	*this << SectionID;

	const auto SizeLocation = Tell( );

	int Size = 0;
	*this << Size;
	const auto SizeStart = Tell( );

	*this << Manager->DestroyedActors;

	const auto FinalPos = Tell( );

	Seek( SizeLocation );
	int FinalSize = FinalPos - SizeStart;
	*this << FinalSize;

	Seek( FinalPos );
}

FArchive& FPersistentActorWriter::operator<<( FSoftObjectPtr &Value )
{
	// Serializes the underlying soft object path. Will NOT go through FPersistentActorWriter::operator<<(UObject*& Obj)
	// Objects in ReferencedObjectList will not be serialized correctly here as they are supposed to be serialized by object index rather than path.
	ensureMsgf( !Value.IsValid( ) || (ObjectToReferenceIndexMap.Find( Value.Get( ) ) == nullptr), TEXT( "Encountered SoftObjectPtr pointing to an object in FPersistentActorWriter::ReferencedObjectList. Will not serialize correctly. - \"%s\"" ), *Value.ToString( ) );

	return FArchiveUObject::SerializeSoftObjectPtr(*this, Value);
}

FArchive& FPersistentActorWriter::operator<<( FSoftObjectPath &Value )
{
	return ArchiveUtilities::SerializeSoftObjectPathWithoutPIEPrefix( *this, Value );
}

FArchive& FPersistentActorWriter::operator<<( UObject *&Res )
{
	// Serialization calls for WeakObjectPtrs and LazyObjectPtrs call into here as well
	const auto FoundReferencedObjectListIndex = ObjectToReferenceIndexMap.Find( Res );
	if (FoundReferencedObjectListIndex != nullptr)
	{
		auto ReferencedObjectListIndex = *FoundReferencedObjectListIndex;
		*this << ReferencedObjectListIndex;
	}
	else
	{
		int32 InvalidIndex = -1;
		*this << InvalidIndex;

		ArchiveUtilities::SerializeObjectPointer( *this, Res );
	}

	return *this;
}

bool FPersistentActorWriter::ShouldIncludeComponent( const UActorComponent *Component ) const
{
	if (Component->IsEditorOnly( ))
		return false; // these aren't gameplay relevant, so skip 'em
	
	if (const auto Persistence = Cast< UPersistenceComponent >( Component ))
		return Persistence->WasSpawned( ); // only keep spawned persistence components since there are no changes to one in maps

	if (!Settings->ShouldPersistType( Component->GetClass( ) ))
		return false;

	if (ComponentFilter && !ComponentFilter( Component ))
		return false;

	return true;
}

void FPersistentActorWriter::RecursiveCollectObjects( UObject *Target, const UPersistenceComponent *Component, int32 CurrentDepth )
{
	if (CurrentDepth >= ArchiveUtilities::MaxRecursionDepth)
	{
		UE_LOGFMT( LogStarfirePersistence, Error, "Persistent Actor Writer::RecursiveCollectObjects exceeded the maximum recursion depth of {0}", ArchiveUtilities::MaxRecursionDepth );
		return;
	}

	// Serialize object with reference collector.
	TArray< UObject* > LocalCollectedReferences;
	FReferenceFinder ObjectReferenceCollector( LocalCollectedReferences, nullptr, false, true, true, true );
	ObjectReferenceCollector.FindReferences( Target );

	const auto StartingListSize = ReferencedObjectList.Num( );
	for (const auto Reference : LocalCollectedReferences)
	{
		if (Reference == nullptr)
			continue;

		if (ObjectToReferenceIndexMap.Contains( Reference ))
			continue; // element already tracked, can skip this time

		const auto ComponentRef = Cast< UActorComponent >( Reference );
		if ((ComponentRef != nullptr) && !ShouldIncludeComponent( ComponentRef ))
			continue;

		const auto bIsInHierarchy = (Reference->GetOuter( ) == Target);
		if (!bIsInHierarchy)
			continue;

		const auto OuterIndex = ObjectToReferenceIndexMap.Find( Reference->GetOuter( ) );
		check( OuterIndex != nullptr ); // should have found the outer by this point

		auto &Entry = ReferencedObjectList.Emplace_GetRef( Reference );

		Entry.OuterIndex = *OuterIndex;

		if (bIsInHierarchy)
		{
			if (Reference->IsA< UActorComponent >( ))
				Entry.SubobjectName = Reference->GetFName( );
			else if (ReferencedObjectList[ *OuterIndex ].Subobjects.Contains( Reference ))
				Entry.SubobjectName = Reference->GetFName( );
		}
		
		FReferenceFinder SubobjectCollector( Entry.Subobjects, Entry.Object, true, true, true, true );
		SubobjectCollector.FindReferences( Entry.Object );

		// Do this after using OuterIndex in case the add causes the reference to be invalidated
		ObjectToReferenceIndexMap.Add( Reference, ReferencedObjectList.Num( ) - 1 );
	}
	const int32 EndingListSize = ReferencedObjectList.Num( );

	// Now recurse down into the hierarchy. This is done in two steps, making the setting of class infos and map indices breadth first
	++CurrentDepth;
	for (int32 idx = StartingListSize; idx < EndingListSize; ++idx)
	{
		UObject *Reference = ReferencedObjectList[ idx ].Object;
		RecursiveCollectObjects( Reference, nullptr, CurrentDepth );
	}
}

// *********************************************************************************************************************
// Persistent Actor Reader Archive
// *********************************************************************************************************************

FPersistentActorReader::FPersistentActorReader( FArchive& InInnerArchive ) :
	FPersistentActorArchiver( InInnerArchive, true )
{
	FArchive::SetIsSaving( false );
}

void FPersistentActorReader::Archive( const UObject *WorldContext )
{
	check( WorldContext != nullptr );
	check( ReferencedObjectList.IsEmpty( ) );
	
	EPersistenceVersion Version;
	*this << Version;

	UWorld* World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	check( World != nullptr );

	Manager = UPersistenceManager::GetSubsystem( WorldContext );

	// Build a mapping of all the subsystems that are persistent
	TMap< UClass*, USubsystem* > PersistentSubsystems;
	for (TObjectIterator< USubsystem > It; It; ++It)
	{
		if (It->GetWorld( ) != World)
			continue;

		if (!It->Implements< UPersistentSubsystem >( ))
			continue;

		PersistentSubsystems.Add( It->GetClass( ), *It );
	}

	*this << ReferencedObjectList;

	// Associate each entry with an object
	// Spawn an Actor, create an object, find an existing actor/component/subsystem
	TArray< FArchivedActor > ActorResults;
	ActorResults.Reserve( ReferencedObjectList.Num( ) );
	for (FPersistentObjectRecord& Entry : ReferencedObjectList)
	{
		// Determine the object that should be the outer (outer should always be at a lower index)
		UObject* Outer = GetTransientPackage();
		const FPersistentObjectRecord* OuterRecord = nullptr;
		if (ReferencedObjectList.IsValidIndex( Entry.OuterIndex ))
		{
			OuterRecord = &ReferencedObjectList[ Entry.OuterIndex ];
			Outer = OuterRecord->Object;

			if (Outer == nullptr)
			{
				const auto& OuterInfo = ReferencedObjectList[ Entry.OuterIndex ];
				UE_LOGFMT( LogStarfirePersistence, Warning, "Unable to construct object of type \"{0}\" in PersistentActorReader. Uses outer of type \"{1}\" which could not be constructed.", Entry.ClassPtr.ToString( ), OuterInfo.ClassPtr.GetAssetName( ) );
				continue;
			}
		}

		if (Entry.PersistentID.IsValid( ) && !Entry.bWasSpawned)
		{
			auto Actor = Manager->FindActor( Entry.PersistentID );
			Entry.Object = Actor.Get( nullptr );
			if (Entry.Object == nullptr)
			{
				UE_LOGFMT( LogStarfirePersistence, Warning, "Unable to find persistent actor of type \"{0}\" with ID \"{1}\" in PersistentActorReader", Entry.ClassPtr.ToString( ), Entry.PersistentID );
			}
			else
			{
				ActorResults.AddUninitialized( );
				ActorResults.Last( ).Actor = Actor.GetValue( );
				ActorResults.Last( ).Type = EArchivedActorType::Updated;
			}

			Entry.bWasConstructed = (Entry.Object != nullptr);
		}
		else if (const auto LoadedClass = Entry.ClassPtr.LoadSynchronous( ))
		{
			if (LoadedClass->IsChildOf< AGameModeBase >( ))
			{
				Entry.Object = World->GetAuthGameMode( );
				Entry.bWasSpawned = false; // technically it was spawned, but *this* process didn't spawn it, and we don't want to call FinishSpawning on the GameState in the next step
				if (Entry.Object == nullptr)
				{
					UE_LOGFMT( LogStarfirePersistence, Error, "Unable to find GameMode while reading from PersistentActorReader." );
					continue;
				}

				if (!Entry.Object->IsA( LoadedClass ))
				{
					Entry.Object = nullptr; // don't try to serialize into a different type
					UE_LOGFMT( LogStarfirePersistence, Warning, "World's Game Mode is no longer the expected type of \"{0}\".", Entry.ClassPtr.ToString( ) );
					continue;
				}

				ActorResults.AddUninitialized( );
				ActorResults.Last( ).Actor = World->GetGameState( );
				ActorResults.Last( ).Type = EArchivedActorType::Updated;
			}
			else if (LoadedClass->IsChildOf< AGameStateBase >( ))
			{
				Entry.Object = World->GetGameState( );
				Entry.bWasSpawned = false; // technically it was spawned, but *this* process didn't spawn it, and we don't want to call FinishSpawning on the GameState in the next step
				if (Entry.Object == nullptr)
				{
					UE_LOGFMT( LogStarfirePersistence, Error, "Unable to find GameState while reading from PersistentActorReader." );
					continue;
				}

				if (!Entry.Object->IsA( LoadedClass ))
				{
					Entry.Object = nullptr; // don't try to serialize into a different type
					UE_LOGFMT( LogStarfirePersistence, Warning, "World's Game State is no longer the expected type of \"{0}\".", Entry.ClassPtr.ToString( ) );
					continue;
				}

				ActorResults.AddUninitialized( );
				ActorResults.Last( ).Actor = World->GetGameState( );
				ActorResults.Last( ).Type = EArchivedActorType::Updated;
			}
			else if (LoadedClass->IsChildOf< AActor >( ))
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.bDeferConstruction = true;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const auto Actor = World->SpawnActor( LoadedClass, &FTransform::Identity, SpawnInfo );
				Entry.Object = Actor;
				
				ActorResults.AddUninitialized( );
				ActorResults.Last( ).Actor = Actor;
				ActorResults.Last( ).Type = EArchivedActorType::Spawned;
			}
			else if (LoadedClass->IsChildOf< UActorComponent >( ))
			{
				const auto ActorOuter = CastChecked< AActor >( Outer ); // if we got one this must be true, right?

				Entry.Object = ArchiveUtilities::FindComponentByName( ActorOuter, Entry.SubobjectName );
				if (Entry.Object == nullptr)
				{
					Entry.Object = NewObject< UObject >( Outer, LoadedClass, Entry.SubobjectName );
					ActorOuter->AddOwnedComponent( CastChecked< UActorComponent >( Entry.Object ) );
				}
			}
			else if (LoadedClass->IsChildOf< USubsystem >( ))
			{
				if (const auto Subsystem = PersistentSubsystems.Find( LoadedClass ))
				{
					Entry.Object = *Subsystem;
				}
				else
				{
					UE_LOGFMT( LogStarfirePersistence, Warning, "Failed to find persistent subsystem of type \"{0}\" in PersistentActorReader.", Entry.ClassPtr.ToString( ) );
					continue;
				}
			}
			else
			{
				if (Entry.SubobjectName.IsNone( ))
				{
					Entry.Object = NewObject< UObject >( Outer, LoadedClass );
				}
				else if (OuterRecord != nullptr)
				{
					Entry.Object = ArchiveUtilities::FindObjectByName( OuterRecord->Subobjects, Entry.SubobjectName );
					if (Entry.Object == nullptr) // must not be a default subobject, so we'll spawn it instead
						Entry.Object = NewObject< UObject >( Outer, LoadedClass );
				}
				else
				{
					UE_LOGFMT( LogStarfirePersistence, Warning, "Failed to find Outer for \"{0}\" sub-object in PersistentActorReader.", Entry.ClassPtr.ToString( ) );
					continue;
				}
			}

			check( Entry.Object != nullptr );
			Entry.bWasConstructed = true;

			FReferenceFinder SubobjectCollector( Entry.Subobjects, Entry.Object, true, true, true, true );
			SubobjectCollector.FindReferences( Entry.Object );
		}
		else
		{
			UE_LOGFMT( LogStarfirePersistence, Error, "Unable to construct object of type \"{0}\" in PersistentActorReader", Entry.ClassPtr.ToString( ) );
		}
	}

#if !UE_BUILD_SHIPPING
	// A helpful debugging view of the serialized objects bucketed by type
	TMap< const UClass*, TArray< UObject* > > DebuggingLookup;
	// Debugging view of the serialized sizes (uncompressed) of the objects in the save
	// Size key is the lower bound log10 of the BlockSize ([0:9] -> 1, [10:99] -> 10, [100:999] -> 100, etc)
	TMap< int, TMap< TSoftClassPtr< UObject >, TArray< int > > > SizeBuckets;
#endif
	// Serialize each of the objects actual properties
	// Done separately so that circular references are serialized properly
	// (since those only need the object to exist, not be filled in)
	TMap< UObject*, FTransform > ActorTransforms;
	for (const FPersistentObjectRecord& Entry : ReferencedObjectList)
	{
		int64 BlockSize = 0;
		*this << BlockSize;

#if !UE_BUILD_SHIPPING
		const auto Log10 = (int)FMath::LogX( 10.0f, BlockSize );
		
		auto& Bucket = SizeBuckets.FindOrAdd( (int)FMath::Pow( 10.0, Log10 ) );
		Bucket.FindOrAdd( Entry.ClassPtr ).Push( BlockSize );
#endif

		if (Entry.Object != nullptr)
		{
#if !UE_BUILD_SHIPPING
			DebuggingLookup.FindOrAdd( Entry.ClassPtr.Get() ).Push( Entry.Object );
#endif

			const int64 BlockStart = Tell( );

			Entry.Object->Serialize( *this );
			
			if (AActor* Actor = Cast< AActor >( Entry.Object ))
			{
				AActor* Owner = nullptr;
				*this << Owner;

				Actor->SetOwner( Owner );

				const auto Component = Actor ? Actor->GetComponentByClass< UPersistenceComponent >( ) : nullptr;
				if ((Component != nullptr) && (Component->bPersistTransform))
				{
					FTransform ActorTransform;
					*this << ActorTransform;

					ActorTransforms.Add( Actor, ActorTransform );
				}
				else
				{
					ActorTransforms.Add( Actor, FTransform::Identity );
				}
			}

			const int64 BlockEnd = Tell( );
			ensureAlways( (BlockEnd - BlockStart) == BlockSize );
		}
		else
		{
			// something's went wrong, skip over this object's data
			Seek( Tell( ) + BlockSize );
		}
	}

	// Finalize each object (depending on the type of the object)
	// Done separately so that references through pointers do have all their data available to use
	for (const FPersistentObjectRecord& Entry : ReferencedObjectList)
	{
		if (Entry.Object == nullptr)
			continue;

		if (const auto Actor = Cast< AActor >( Entry.Object ))
		{
			const auto& ActorTransform = ActorTransforms.FindRef( Entry.Object );

			if (Entry.bWasSpawned)
				Actor->FinishSpawning( ActorTransform );
			else
				Actor->SetActorTransform( ActorTransform );

			if (const auto Component = Actor->GetComponentByClass< UPersistenceComponent >())
				Component->OnPostDeserialize.Broadcast( );
		}
		else if (const auto Component = Cast< UActorComponent >( Entry.Object ))
		{
			if (!Component->IsRegistered( ))
				Component->RegisterComponent( );
		}
		else if (const auto Interface = Cast< IPersistentSubsystem >( Entry.Object ))
		{
			Interface->PostDeserialize( );
		}
	}

	// Reset this in case Archive is called repeatedly
	ReferencedObjectList.Empty( );

	while (!AtEnd( ))
	{
		ESectionID SectionID;
		*this << SectionID;

		int32 Size = 0;
		*this << Size;

		switch (SectionID)
		{
			case ESectionID::DestroyedActors:
				SerializeDestroyedActors(  );
				break;

			default:
				UE_LOGFMT( LogStarfirePersistence, Warning, "PersistentActorReader found unknown SectionID {0}. Skipping {1} bytes", static_cast< int >(SectionID), Size );
				Seek( Tell( ) + Size );
				break;
		}
	}

	for (const FPersistentObjectRecord& Entry : ReferencedObjectList)
	{
		if (const auto Actor = Cast< AActor >( Entry.Object ))
		{
			if (const auto Component = Actor->GetComponentByClass< UPersistenceComponent >())
				Component->OnPostArchive.Broadcast( ActorResults );
		}
		else if (const auto Interface = Cast< IPersistentSubsystem >( Entry.Object ))
		{
			Interface->PostArchiveLoad( ActorResults );
		}
	}
}

FArchive& FPersistentActorReader::operator<<( UObject *&Res )
{
	int32 RefIndex;
	*this << RefIndex;

	if (RefIndex == -1) // Not an indexed object, serialize the pointer directly
	{
		ArchiveUtilities::SerializeObjectPointer( *this, Res, Manager );
	}
	else if (ReferencedObjectList.IsValidIndex( RefIndex )) // grab from the known object list
	{
		Res = ReferencedObjectList[ RefIndex ].Object;
	}
	else // everything should work out as one of those two things
	{
		UE_LOGFMT( LogStarfirePersistence, Warning, "Unable to deserialize reference index {0}. Entries contain {1} elements.", RefIndex, ReferencedObjectList.Num( ) );
		Res = nullptr;
	}

	return *this;
}

void FPersistentActorReader::SerializeDestroyedActors( )
{
	*this << Manager->DestroyedActors;

	for (const auto &ID : Manager->DestroyedActors)
	{
		const auto Actor = Manager->FindActor( ID );

		if (const auto ActorPtr = Actor.Get( nullptr ))
			ActorPtr->Destroy( );
	}
}
