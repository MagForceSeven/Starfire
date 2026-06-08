
#include "ActorViewModels/ActorVMResolver.h"

#include "ActorViewModels/ActorVMBase.h"
#include "ActorViewModels/ActorVMSingleton.h"
#include "ActorViewModels/ActorWidgetInterface.h"
#include "ActorViewModels/ActorVMUtilities.h"

// UMG
#include "Blueprint/UserWidget.h"

// Engine
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorVMResolver)

UActorVMBase * UActorVMResolver::FindOrCreateVM( AActor *Actor, const UClass *Class )
{
	return UActorVMUtilities::FindOrCreateVM( Actor, Class );
}

UObject* UActorVMResolver::CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const
{
	if (!UserWidget->Implements< UActorWidgetInterface >( ))
		return nullptr;

	const auto Actor = IActorWidgetInterface::Execute_GetActor( UserWidget );
	if (Actor == nullptr)
		return nullptr;

	return FindOrCreateVM( Actor, ExpectedType );
}

#if WITH_EDITOR
bool UActorVMResolver::DoesSupportViewModelClass( const UClass* Class ) const
{
	if (Class->IsChildOf< UActorVMSingleton >( ))
		return false;
	
	return Class->IsChildOf< UActorVMBase >( );
}
#endif

UObject* USingletonActorVMResolver::CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const
{
	if (!ensureAlways( SingletonActorClass ) )
		return nullptr;
	
	TActorIterator< AActor > It( UserWidget->GetWorld( ), SingletonActorClass );
	if (!It)
		return nullptr;

	const auto ActorInstance = *It;
	++It;
	if (It)
	{
		UE_LOG( LogActorVM, Warning, TEXT( " Singleton Actor VM Resolver found multiple instances of class '%s'." ), *SingletonActorClass->GetName( ) );
		return nullptr;
	}

	return FindOrCreateVM( ActorInstance, ExpectedType );
}

UObject* USingletonVMResolver::CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const
{
	// Since there should only ever be one of this type of VM, we'll cache them all on the Game State like any other ActorVM

	const auto GameState = UGameplayStatics::GetGameState( UserWidget );
	if (!ensureAlways( GameState ))
		return nullptr;

	return FindOrCreateVM( GameState, ExpectedType );
}

#if WITH_EDITOR
bool USingletonVMResolver::DoesSupportViewModelClass( const UClass* Class ) const
{
	return Class->IsChildOf< UActorVMSingleton >( );
}
#endif