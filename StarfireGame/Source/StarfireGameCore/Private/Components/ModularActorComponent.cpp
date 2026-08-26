
#include "Components/ModularActorComponent.h"

#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularActorComponent)

void UModularActorComponent::OnRegister( )
{
	Super::OnRegister( );

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(GetOwner( ));
}

void UModularActorComponent::BeginPlay( )
{
	Super::BeginPlay( );

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(GetOwner( ), UGameFrameworkComponentManager::NAME_GameActorReady);
}

void UModularActorComponent::EndPlay( EEndPlayReason::Type EndPlayReason )
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(GetOwner( ));

	Super::EndPlay( EndPlayReason );
}
