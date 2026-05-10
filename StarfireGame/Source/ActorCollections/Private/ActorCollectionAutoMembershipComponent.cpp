
#include "ActorCollectionAutoMembershipComponent.h"

#include "ActorCollectionUtilities.h"
#include "ActorCollectionSingleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorCollectionAutoMembershipComponent)

void UActorCollectionAutoMembershipComponent::BeginPlay( )
{
	Super::BeginPlay( );

	auto CollectionClass = GetAssociatedCollectionClass_BP( );
	if (CollectionClass == nullptr)
		CollectionClass = GetAssociatedCollectionClass( );

	if (ensureAlways( CollectionClass != nullptr ))
		return;

	const auto Singleton = UActorCollectionUtilities::GetOrCreateSingletonCollection( this, CollectionClass );
	check( Singleton != nullptr );

	Singleton->AddToCollection( GetOwner( ) );
}
