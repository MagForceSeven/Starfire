
#pragma once

#include "CoreMinimal.h"

#include "PersistenceShared.generated.h"

// The different ways Actors may have been updated that need to be responded to
UENUM( BlueprintType )
enum class EArchivedActorType : uint8
{
	None,		// default (unknown)
	Spawned,	// Actor is newly created
	Updated,	// Actor was modified
	Destroyed,	// Actor was marked for destruction
};

// Tracking data for the actor changes that occurred during the loading of an archive
USTRUCT( BlueprintType )
struct FArchivedActor
{
	GENERATED_BODY( )
public:
	// Way that the actor was affected by the serialization process
	UPROPERTY( BlueprintReadOnly )
	EArchivedActorType Type = EArchivedActorType::None;

	// The actor that was modified by serialization
	UPROPERTY( BlueprintReadOnly )
	TObjectPtr< AActor > Actor = nullptr;
};