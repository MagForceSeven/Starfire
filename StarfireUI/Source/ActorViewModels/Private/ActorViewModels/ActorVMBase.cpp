
#include "ActorViewModels/ActorVMBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ActorVMBase)

DEFINE_LOG_CATEGORY(LogActorVM);

void UActorVMBase::OnCreate( AActor *Actor )
{
	if (!ensureAlways( Actor != nullptr ))
		return;

	AssociatedActor = Actor;

	HandleOnCreate( Actor );
	ReceiveOnCreate( Actor );
}

void UActorVMBase::OnDestroy( )
{
	ReceiveOnDestroy( );
	HandleOnDestroy( );
}
