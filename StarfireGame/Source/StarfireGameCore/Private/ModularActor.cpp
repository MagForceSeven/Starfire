
#include "ModularActor.h"

#include "Components/ModularActorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularActor)

AModularActor::AModularActor( )
{
	PrimaryActorTick.bStartWithTickEnabled = false;

	CreateDefaultSubobject< UModularActorComponent >( FName("ModularActor") );
}
