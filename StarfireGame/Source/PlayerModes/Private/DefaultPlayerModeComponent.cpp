
#include "DefaultPlayerModeComponent.h"

#include "PlayerModeStack.h"
#include "PlayerModeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DefaultPlayerModeComponent)

void UDefaultPlayerModeComponent::BeginPlay( )
{
	Super::BeginPlay( );

	if (DefaultPlayerMode)
	{
		const auto Stack = UPlayerModeStack::GetSubsystem( GetOwner( ) );

		PushDefaultMode( Stack );

		Stack->OnPlayerModeRemoved.AddUObject( this, &UDefaultPlayerModeComponent::OnModeRemoved );
	}
}

void UDefaultPlayerModeComponent::SetDefaultPlayerMode( const TSubclassOf<APlayerModeBase> &Mode )
{
	if (!ensureAlways( !HasBegunPlay( ) ))
		return; // this should only be set from the constructor
	
	DefaultPlayerMode = Mode;
}

void UDefaultPlayerModeComponent::PushDefaultMode( UPlayerModeStack *Stack )
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;

	const auto Mode = GetWorld( )->SpawnActor< APlayerModeBase >( DefaultPlayerMode, SpawnParams );

	Stack->PushMode( Mode );
}

void UDefaultPlayerModeComponent::OnModeRemoved( UPlayerModeStack *Stack, APlayerModeBase *Mode )
{
	if (Stack->GetCurrentMode( ) == nullptr)
		PushDefaultMode( Stack );
}
