
#include "Subsystems/GameFeatureWorldSubsystem.h"

#include "GameFeatureSubsystems_SubsystemManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureWorldSubsystem)

bool UGameFeatureWorldSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ))
		return false;
	
	return Super::ShouldCreateSubsystem( Outer );
}

void UGameFeatureWorldSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	ensureAlwaysMsgf( UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ), TEXT( "Game Feature Subsystem created while corresponding feature is not active. Forget to call Super::ShouldCreateSubsystem?" ) );
}

void UGameFeatureWorldSubsystem::Deinitialize( )
{
	Super::Deinitialize( );
}

bool UGameFeatureTickableWorldSubsystem::ShouldCreateSubsystem( UObject *Outer ) const
{
	if (!UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ))
		return false;
	
	return Super::ShouldCreateSubsystem( Outer );
}

void UGameFeatureTickableWorldSubsystem::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	ensureAlwaysMsgf( UGameFeatureSubsystems_FeatureModules::ShouldAllowFeatureSubsystem( GetClass( ) ), TEXT( "Game Feature Subsystem created while corresponding feature is not active. Forget to call Super::ShouldCreateSubsystem?" ) );
}

void UGameFeatureTickableWorldSubsystem::Deinitialize( )
{
	Super::Deinitialize( );
}
