
#pragma once

#include "UObject/Interface.h"

#include "PersistentSubsystemInterface.generated.h"

struct FArchivedActor;

// Static type identifying subsystems that are persisted through PersistentActorArchiver
UINTERFACE( MinimalAPI, meta = (CannotImplementInterfaceInBlueprint) )
class UPersistentSubsystem : public UInterface
{
	GENERATED_BODY()
};

// Interface identifying subsystems that are persisted through PersistentActorArchiver
class STARFIREPERSISTENCE_API IPersistentSubsystem
{
	GENERATED_BODY( )
public:
	// Hook for the subsystem to do work prior to the serializing to an archive
	virtual void PreSerialize( void ) { }

	// Hook for the subsystem to do work after serialization from an archive
	virtual void PostDeserialize( void ) { }
	
	// Hook for the subsystem to respond to the completion of the loading process (somewhat analogous to BeginPlay or OnRegisterComponent)
	virtual void PostArchiveLoad( const TArray< FArchivedActor >& ActorChanges ) {  }
};