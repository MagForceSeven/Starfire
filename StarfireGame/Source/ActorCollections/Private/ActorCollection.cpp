
#include "ActorCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorCollection)

void AActorCollection::AddToCollection( AActor *Actor )
{
	AddActor( Actor );
}

void AActorCollection::RemoveFromCollection( AActor *Actor )
{
	RemoveActor( Actor );
}

void AActorCollection::RemoveAll( )
{
	auto &Collection = GetMembers( );
	while (!Collection.IsEmpty( ))
		RemoveActor( Collection.Top( ) );
}