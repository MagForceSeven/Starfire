
#include "ArchiveUtilities.h"

#include "PersistenceComponent.h"
#include "PersistenceManager.h"
#include "Module/StarfirePersistence.h"

// Core
#include "Logging/StructuredLog.h"

namespace ArchiveUtilities
{
	FArchive& SerializeSoftObjectPathWithoutPIEPrefix( FArchive &Ar, FSoftObjectPath& SoftPath )
	{
#if WITH_EDITOR
		if (Ar.IsSaving( ))
		{
			const FString AssetPath = SoftPath.GetAssetPathString( );
			if (AssetPath.Contains( PLAYWORLD_PACKAGE_PREFIX ))
			{
				FSoftObjectPath PathWithoutPIEPrefix( FTopLevelAssetPath( UWorld::RemovePIEPrefix( AssetPath ) ), SoftPath.GetSubPathString( ) );
				PathWithoutPIEPrefix.SerializePath( Ar );
			}
			else
			{
				// Path does not have a PIE prefix so just serialize it normally
				SoftPath.SerializePath( Ar );
			}
		}
		else
#endif
		{
			SoftPath.SerializePath( Ar );
		}

		return Ar;
	}

	FArchive& SerializeObjectPointer( FArchive &Ar, UObject* &Obj, const UPersistenceManager *Manager, bool bLoadIfFindFails )
	{
		// Soft object path that denotes a persistent actor GUID lookup
		static FSoftObjectPath PersistentActorSentinel( "/PA" );

		if (Ar.IsSaving( ))
		{
			const auto Actor = Cast< AActor >( Obj );
			const auto Component = Actor ? Actor->GetComponentByClass< UPersistenceComponent >( ) : nullptr;
			
			if (Component != nullptr)
			{
				PersistentActorSentinel.SerializePath( Ar );

				FGuid ID = Component->GetGuid( );
				Ar << ID;
			}
			else
			{
				FSoftObjectPath SoftPath( Obj );
				SerializeSoftObjectPathWithoutPIEPrefix( Ar, SoftPath );
			}
		}
		else
		{
			FSoftObjectPtr SoftPtr;
			Ar << SoftPtr.GetUniqueID( );

			if (SoftPtr.ToSoftObjectPath( ) == PersistentActorSentinel)
			{
				FGuid ID;
				Ar << ID;

				const auto Actor = Manager ? Manager->FindActor( ID ) : TOptional< AActor* >( );
				if (Actor.IsSet( ))
				{
					Obj = Actor.GetValue( );
				}
				else
				{
					Obj = nullptr;
					UE_LOGFMT( LogStarfirePersistence, Error, "Persistent Actor not found while loading with an ArchiveUtilities::SerializeObjectPointer without an Actor<>Guid mapping!" );
				}
			}
			else if (SoftPtr.IsValid( ))
			{
				Obj = SoftPtr.Get( );
			}
			else if (bLoadIfFindFails && SoftPtr.ToSoftObjectPath( ).IsAsset( ))
			{
				Obj = SoftPtr.LoadSynchronous( );

				if (Obj == nullptr)
				{
					const auto AssetPath = SoftPtr.ToSoftObjectPath( ).GetAssetPathString( );
					UE_LOGFMT( LogStarfirePersistence, Error, "Could not load \"{0}\" in ArchiveUtilities::SerializeObjectPointer with bLoadIfFindFails == true!", AssetPath );
				}
			}
			else
			{
				Obj = nullptr;

				if (!SoftPtr.IsNull( ))
				{
					const auto Path = SoftPtr.ToSoftObjectPath( ).ToString( );
					UE_LOGFMT( LogStarfirePersistence, Verbose, "Could not find \"{0}\" in ArchiveUtilities::SerializeObjectPointer!", Path );
				}
			}
		}

		return Ar;
	}

	UActorComponent* FindComponentByName( const AActor *Actor, const FName &Name )
	{
		for (const auto C : Actor->GetComponents( ))
		{
			if (C == nullptr)
				continue;

			if (C->GetFName( ) == Name)
				return C;
		}
	
		return nullptr;
	}

	UObject* FindObjectByName( const TArray< UObject* > &Objects, const FName &Name )
	{
		for (const auto O : Objects)
		{
			if (O == nullptr)
				return nullptr;
			
			if (O->GetFName( ) == Name)
				return O;
		}

		return nullptr;
	}
	
}
