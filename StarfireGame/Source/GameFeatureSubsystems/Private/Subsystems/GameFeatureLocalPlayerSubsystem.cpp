
#include "Subsystems/GameFeatureLocalPlayerSubsystem.h"

#include "GameFeatureSubsystems_SubsystemManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureLocalPlayerSubsystem)

bool UGameFeatureLocalPlayerSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ))
		return false;
	
	return Super::ShouldCreateSubsystem( Outer );
}

void UGameFeatureLocalPlayerSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	ensureAlwaysMsgf( UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ), TEXT( "Game Feature Subsystem created while corresponding feature is not active. Forget to call Super::ShouldCreateSubsystem?" ) );
}

void UGameFeatureLocalPlayerSubsystem::Deinitialize( )
{
	Super::Deinitialize( );
}
