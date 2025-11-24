
#include "PersistenceComponent.h"

#include "PersistenceManager.h"

// Core UObject
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PersistenceComponent)

UPersistenceComponent::UPersistenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FGuid UPersistenceComponent::GetGuid() const
{
	if (!PersistentGuid.IsValid())
	{
		ensureAlwaysMsgf(bSpawned, TEXT("Editor placed Actor did not save Persistent ID properly."));
		PersistentGuid = FGuid::NewGuid();
	}
	
	return PersistentGuid;
}

void UPersistenceComponent::PreSave( FObjectPreSaveContext SaveContext )
{
	Super::PreSave( SaveContext );

	// In the editor, we'll copy the ActorGuid that is used for managing Actors
	// That GUID seems to work in all cases as expected and in a way that's not easy to replicate with custom member/code
	// GUIDs for runtime-spawned actors will be created on demand
#if WITH_EDITORONLY_DATA
	if (!IsTemplate( ))
	{
		PersistentGuid = GetOwner( )->GetActorInstanceGuid(  );
		bSpawned = false;
	}
#endif
}

void UPersistenceComponent::PostDuplicate( bool bDuplicateForPIE )
{
	Super::PostDuplicate( bDuplicateForPIE );

	// In PIE, PreSave doesn't get called. So unsaved persistence components will have the wrong values
	// Any duplication other than PIE should result in resetting the properties since they are DuplicateTransient
#if WITH_EDITORONLY_DATA
	if (bDuplicateForPIE)
	{
		PersistentGuid = GetOwner( )->GetActorInstanceGuid( );
		bSpawned = false;
	}
#endif
}

void UPersistenceComponent::BeginPlay( )
{
	Super::BeginPlay( );

	if (bSpawned)
	{
		const auto Manager = UPersistenceManager::GetSubsystem( this );
		Manager->AddSpawnedActor( GetOwner( ), PersistentGuid );
	}
}

void UPersistenceComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if (!bSpawned && bPersistDestruction && (EndPlayReason == EEndPlayReason::Destroyed))
	{
		const auto Manager = UPersistenceManager::GetSubsystem( this );
		Manager->TrackActorAsDestroyed( PersistentGuid );
	}
	else if (bSpawned)
	{
		const auto Manager = UPersistenceManager::GetSubsystem( this );
		Manager->RemoveSpawnedActor( PersistentGuid );
	}

	Super::EndPlay( EndPlayReason );
}
