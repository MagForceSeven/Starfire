
#include "Subsystems/GameFeatureGameInstanceSubsystem.h"

#include "GameFeatureSubsystems_SubsystemManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureGameInstanceSubsystem)

bool UGameFeatureGameInstanceSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ))
		return false;
	
	return Super::ShouldCreateSubsystem( Outer );
}

void UGameFeatureGameInstanceSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	ensureAlwaysMsgf( UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ), TEXT( "Game Feature Subsystem created while corresponding feature is not active. Forget to call Super::ShouldCreateSubsystem?" ) );
}

void UGameFeatureGameInstanceSubsystem::Deinitialize( )
{
	Super::Deinitialize( );
}
