
#include "DataStoreSingleton.h"

#include "PersistentDataStore.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreSingleton)

void ADataStoreSingleton::PostRegisterAllComponents( )
{
	Super::PostRegisterAllComponents( );

	const auto Subsystem = UPersistentDataStore::GetSubsystem( this );
	check( Subsystem != nullptr );

	Subsystem->AddSingleton( this );
}

void ADataStoreSingleton::Destroyed( )
{
	const auto Subsystem = UPersistentDataStore::GetSubsystem( this );
	check( Subsystem != nullptr );

	Subsystem->RemoveSingleton( this );
	
	Super::Destroyed( );
}

#if WITH_EDITOR
FString ADataStoreSingleton::GetCustomActorLabel_Implementation( ) const
{
	return GetClass( )->GetDisplayNameText( ).ToString( );
}

FString ADataStoreSingleton::GetCustomOutlinerFolder_Implementation( ) const
{
	return "Singletons";
}
#endif