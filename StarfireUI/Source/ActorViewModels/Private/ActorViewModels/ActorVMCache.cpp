
#include "ActorViewModels/ActorVMCache.h"

#include "ActorViewModels/ActorVMBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorVMCache)

UActorVMBase* UActorVMCacheComponent::FindOrCreateVM( const UClass *Class )
{
	check( Class != nullptr );
	check( Class->IsChildOf< UActorVMBase >( ) );

	for (const auto &VM : CachedVMs)
	{
		if (VM->IsA( Class ))
			return VM;
	}

	const auto NewVM = NewObject< UActorVMBase >( this, Class );
	NewVM->OnCreate( GetOwner( ) );

	CachedVMs.Push( NewVM );

	return NewVM;
}

void UActorVMCacheComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	for (const auto &VM : CachedVMs)
		VM->OnDestroy( );

	CachedVMs.Empty( );
	
	Super::EndPlay( EndPlayReason );
}
