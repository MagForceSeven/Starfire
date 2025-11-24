
#include "DataStoreSingleton.h"

#include "PersistentDataStore.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreSingleton)

void ADataStoreSingleton::PostActorCreated( )
{
	Super::PostActorCreated( );

	if (const auto Subsystem = UPersistentDataStore::GetSubsystem( this ))
		Subsystem->AddSingleton( this );
}

void ADataStoreSingleton::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ((EndPlayReason == EEndPlayReason::Destroyed) || (EndPlayReason == EEndPlayReason::RemovedFromWorld))
	{
		if (const auto Subsystem = UPersistentDataStore::GetSubsystem( this ))
			Subsystem->RemoveSingleton( this );
	}
	
	Super::EndPlay( EndPlayReason );
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