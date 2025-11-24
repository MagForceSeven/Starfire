
#pragma once

class UPersistenceManager;

namespace ArchiveUtilities
{
	constexpr int32 MaxRecursionDepth = 100;

	// Serializes a soft object path. When saving the PIE prefix will be removed if necessary. Use when saving in-game objects that may be loaded in a non-PIE game.
	FArchive& SerializeSoftObjectPathWithoutPIEPrefix( FArchive &Ar, FSoftObjectPath& SoftPath );

	// Serializes an object pointer. Serializes PersistentObjects by GUID. Serializes all other objects as a soft object path.
	FArchive& SerializeObjectPointer( FArchive &Ar, UObject* &Obj, const UPersistenceManager *Manager = nullptr, bool bLoadIfFindFails = false );

	// Utility for finding a component on an Actor by name - how is this not built in to Actor?
	[[nodiscard]] UActorComponent* FindComponentByName( const AActor *Actor, const FName &Name );

	// Utility for finding an object with a specific name from a collection of objects
	[[nodiscard]] UObject* FindObjectByName( const TArray< UObject* > &Objects, const FName &Name );

}
