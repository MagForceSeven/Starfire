
#include "DataStoreSingleton.h"

#include "PersistentDataStore.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreSingleton)

void ADataStoreSingleton::PostRegisterAllComponents( )
{
	Super::PostRegisterAllComponents( );

	if (const auto Subsystem = UPersistentDataStore::GetSubsystem( this ))
		Subsystem->AddSingleton( this );
}

void ADataStoreSingleton::Destroyed( )
{
	if (const auto Subsystem = UPersistentDataStore::GetSubsystem( this ))
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