
#include "ActorViewModels/ActorVMUtilities.h"

#include "ActorViewModels/ActorVMBase.h"
#include "ActorViewModels/ActorVMSingleton.h"
#include "ActorViewModels/ActorVMCache.h"

// Engine
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorVMUtilities)

UActorVMBase* UActorVMUtilities::FindOrCreateVM( AActor *Actor, TSubclassOf< UActorVMBase > VMType )
{
	// Remap singletons to the GameState as the owner
	if ((VMType != nullptr) && VMType->IsChildOf< UActorVMSingleton >( ))
		Actor = UGameplayStatics::GetGameState( Actor );

	return FindOrCreateVM( Actor, VMType.Get( ) );
}

UActorVMBase* UActorVMUtilities::FindOrCreateSiblingVM( const UActorVMBase *ActorVM, TSubclassOf< UActorVMBase > VMType )
{
	if (!ensureAlways( ActorVM != nullptr ))
		return nullptr;

	// Remap singletons to the GameState as the owner
	if ((VMType != nullptr) && VMType->IsChildOf< UActorVMSingleton >( ))
		return FindOrCreateVM( UGameplayStatics::GetGameState( ActorVM->GetActor( ) ), VMType.Get( ) );
	
	return FindOrCreateVM( ActorVM->GetActor( ), VMType.Get( ) );
}

UActorVMBase * UActorVMUtilities::FindOrCreateVM( const UActorComponent *Component, TSubclassOf<UActorVMBase> VMType )
{
	if (!ensureAlways( Component != nullptr ))
		return nullptr;

	return FindOrCreateVM( Component->GetOwner( ), VMType );
}

UActorVMBase* UActorVMUtilities::FindOrCreateVM( AActor *Actor, const UClass *Class )
{
	if (!ensureAlways( Actor != nullptr ))
		return nullptr;
	if (!ensureAlways( Class != nullptr ))
		return nullptr;
	if (!ensureAlways( Class->IsChildOf< UActorVMBase >( ) ))
		return nullptr;
	
	auto VMCache = Actor->GetComponentByClass< UActorVMCacheComponent >( );
	if (VMCache == nullptr)
	{
		VMCache = NewObject< UActorVMCacheComponent >( Actor );
		Actor->AddOwnedComponent( VMCache );
		VMCache->RegisterComponent( );
	}

	return VMCache->FindOrCreateVM( Class );
}
