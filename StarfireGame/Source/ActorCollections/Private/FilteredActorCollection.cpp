
#include "FilteredActorCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FilteredActorCollection)

void AFilteredActorCollection::BeginPlay( )
{
	Super::BeginPlay( );

	ensureAlwaysMsgf( SourceCollection != nullptr, TEXT( "No Source Collection to create a filter from. Spawn FilterCollections from ActorCollectionManager." ) );
}

void AFilteredActorCollection::EndPlay( EEndPlayReason::Type Param )
{
	SourceCollection->OnActorAdded.RemoveAll( this );
	SourceCollection->OnActorRemoved.RemoveAll( this );
	
	Super::EndPlay( Param );
}

void AFilteredActorCollection::RefreshActorStatus( AActor *Actor )
{
	if (!ensureAlways( SourceCollection->Contains( Actor )))
		return;

	const auto bShouldInclude = IncludeInCollection( Actor );
	const auto bIsIncluded = Contains( Actor );

	if (bShouldInclude == bIsIncluded)
		return;

	if (bShouldInclude)
		AddActor( Actor );
	else
		RemoveActor( Actor );
}

void AFilteredActorCollection::InitSourceCollection( AActorCollectionBase *Collection )
{
	check( Collection != nullptr );
	
	SourceCollection = Collection;

	const auto OnSourceDestroyed = FOnActorDestroyed::FDelegate::CreateWeakLambda( this,
		[ this ]( AActor *Actor ) -> void
		{
			if (Actor == SourceCollection)
				Destroy( );
		} );

	SourceCollection->GetWorld( )->AddOnActorDestroyedHandler( OnSourceDestroyed );

	SourceCollection->OnActorAdded.AddUObject( this, &AFilteredActorCollection::AddedToSourceCollection );
	SourceCollection->OnActorRemoved.AddUObject( this, &AFilteredActorCollection::RemovedFromSourceCollection );
}

void AFilteredActorCollection::AddedToSourceCollection( AActor *NewActor, AActorCollectionBase *Collection )
{
	if (IncludeInCollection( NewActor ))
		AddActor( NewActor );
}

void AFilteredActorCollection::RemovedFromSourceCollection( AActor *RemovedActor, AActorCollectionBase *Collection )
{
	if (!Contains( RemovedActor ))
		return;

	RemoveActor( RemovedActor );
}
