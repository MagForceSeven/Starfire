
#include "ActorCollectionBase.h"

#include "ActorCollectionMembershipComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorCollectionBase)

AActorCollectionBase::AActorCollectionBase( )
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AActorCollectionBase::AddActor( AActor *Actor )
{
	if (!ensureAlways( IsValid( Actor ) ))
		return;

	if (!ensureAlways( !CollectionMembers.Contains( Actor ) ))
		return;

	if (ChangeInProgress)
	{
		PendingChanges.Push( { true, Actor } );
		return;
	}

	ChangeInProgress = true;

	AddActor_Internal( Actor );

	ProcessPendingChanges( );

	ChangeInProgress = false;
}

void AActorCollectionBase::RemoveActor( AActor *Actor )
{
	if (!ensureAlways( IsValid( Actor ) ))
		return;

	if (!ensureAlways( CollectionMembers.Contains( Actor ) ))
		return;

	if (ChangeInProgress)
	{
		PendingChanges.Push( { false, Actor } );
		return;
	}

	ChangeInProgress = true;

	RemoveActor_Internal( Actor );

	ProcessPendingChanges( );

	ChangeInProgress = false;
}

void AActorCollectionBase::AddActor_Internal( AActor *Actor )
{
	auto MembershipComponent = Actor->FindComponentByClass< UActorCollectionMembershipComponent >( );
	if (MembershipComponent == nullptr)
	{
		MembershipComponent = NewObject< UActorCollectionMembershipComponent >( Actor );
		Actor->AddOwnedComponent( MembershipComponent );
		MembershipComponent->RegisterComponent( );
	}

	if (ensureAlways( MembershipComponent != nullptr ))
		MembershipComponent->Collections.Push( this );
	
	CollectionMembers.Push( Actor );

	HandleActorAdded( Actor );
	OnActorAdded.Broadcast( Actor, this );
	OnActorAdded_BP.Broadcast( Actor, this );
}

void AActorCollectionBase::RemoveActor_Internal( AActor *Actor )
{
	const auto MembershipComponent = Actor->FindComponentByClass< UActorCollectionMembershipComponent >( );
	if(ensureAlways( MembershipComponent != nullptr ))
		MembershipComponent->Collections.Remove( this );

	CollectionMembers.Remove( Actor );

	HandleActorRemoved( Actor );
	OnActorRemoved.Broadcast( Actor, this );
	OnActorRemoved_BP.Broadcast( Actor, this );
}

void AActorCollectionBase::ProcessPendingChanges( )
{
	while (!PendingChanges.IsEmpty( ))
	{
		auto& [ToAdd, Actor] = PendingChanges[ 0 ];
		PendingChanges.RemoveAt( 0 );

		if (ToAdd)
			AddActor_Internal( Actor );
		else
			RemoveActor_Internal( Actor );
	}
}