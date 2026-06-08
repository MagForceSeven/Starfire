
#include "Components/GameModeViewModels.h"

#include "ActorViewModels/ActorVMUtilities.h"
#include "ActorViewModels/ActorVMSingleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameModeViewModels)

void UGameModeViewModels::BeginPlay( )
{
	Super::BeginPlay( );

	for (const auto &Type : DefaultViewModels)
	{
		if (!IsValid( Type ))
			continue;
		
		(void)UActorVMUtilities::FindOrCreateVM( this, Type );
	}
}
