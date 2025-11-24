
#pragma once

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

class UPersistenceComponent;
class UPersistenceManager;
class UStarfirePersistenceSettings;

// A shared base for archivers dealing with Persistent Actors & Subsystems
class FPersistentActorArchiver : public FObjectAndNameAsStringProxyArchive
{
public:
	FPersistentActorArchiver( FArchive &InInnerArchive, bool bLoadIfFindFails );

protected:
	// Information about an individual object that should be included in the archive
	struct FPersistentObjectRecord
	{
		FPersistentObjectRecord( ) = default;
		explicit FPersistentObjectRecord( UObject* InObject ) : ClassPtr( InObject->GetClass( ) ), Object( InObject ) { }

		// The type of object
		TSoftClassPtr< UObject > ClassPtr;

		// The index (into ReferencedObjectList) of the Object's Outer
		int OuterIndex = -1;

		// The persistent ID of the Object (if it's a persistent thing)
		FGuid PersistentID;

		// The object name (if it's a component/subobject)
		FName SubobjectName;

		// The object to serialize into/from
		UObject* Object = nullptr;

		// Whether this object has been constructed (or it was attempted and failed)
		bool bWasConstructed = false;

		// For Actors, whether this object is found in the level or spawned dynamically
		bool bWasSpawned = false;

		// The subobjects owned by this one, used to map to default subobjects instead of trying to respawn them
		TArray< UObject* > Subobjects;

		friend FArchive& operator<<( FArchive &Ar, FPersistentObjectRecord &R )
		{
			Ar << R.ClassPtr;
			Ar << R.OuterIndex;
			Ar << R.PersistentID;
			Ar << R.SubobjectName;
			Ar << R.bWasSpawned;

			return Ar;
		}
	};
	
	// A collection of the objects to serialize to or create from the provided archive
	TArray< FPersistentObjectRecord > ReferencedObjectList;
};

// Archiver that can serialize all (or some set) of actors into an archive
class STARFIREPERSISTENCE_API FPersistentActorWriter : public FPersistentActorArchiver
{
public:
	explicit FPersistentActorWriter( FArchive& InInnerArchive );
	
	// Optional method to filter the Objects that should be included in the archive
	// Function should return true if the Actor should be included in the archive
	// This is for top level filtering of Actors w/ Persistence Components and Subsystems implementing the Persistent Subsystem interface
	TFunction< bool (const UObject*) > ObjectFilter;

	// Optional method to filter the Actor Components that should be included in the archive
	// Function should return true if the component should be included in the archive
	// This is in addition and after filtering done through UStarfirePersistenceSettings::ComponentsToPersist
	TFunction< bool (const UActorComponent*) > ComponentFilter;

	// Archive all Actors with a PersistenceComponent & all Subsystem implementing the PersistentSubsystem interface
	void Archive( const UObject* WorldContext );
	// Archive a specific set of Objects
	void Archive( const TArray< UObject* > &Objects );

	// FArchive API
	FArchive& operator<<( FSoftObjectPtr& Value ) override;
	FArchive& operator<<( FSoftObjectPath& Value ) override;
	FArchive& operator<<( UObject*& Res ) override;
	
private:
	// Wrapper to check components for inclusion in the Archive
	[[nodiscard]] bool ShouldIncludeComponent( const UActorComponent* Component ) const;

	// Add objects to the ReferencedObjectList based on Target, and then recursively treat those objects as Target
	void RecursiveCollectObjects( UObject* Target, const UPersistenceComponent* Component, int32 CurrentDepth );

	// Lookup table of the objects that have been added to the List and where they are in the array
	TMap< UObject*, int32 > ObjectToReferenceIndexMap;

	// Cached reference to the config settings for data persistence
	const UStarfirePersistenceSettings *Settings;

	// Add data for destroyed actors to the archive data
	void SerializeDestroyedActors( UPersistenceManager *Manager );
};

// Archiver that can deserializer actors from an archive
class STARFIREPERSISTENCE_API FPersistentActorReader : public FPersistentActorArchiver
{
public:
	explicit FPersistentActorReader( FArchive &InInnerArchive );

	// Use the Archive to restore Actor/Object/Subsystem state
	void Archive( const UObject* WorldContext );

	// FArchive API
	FArchive& operator<<( UObject*& Res ) override;

private:
	// Cached reference to the config settings for data persistence
	UPersistenceManager *Manager = nullptr;

	// Read data for destroyed actors from the archive data
   	void SerializeDestroyedActors( void );
};