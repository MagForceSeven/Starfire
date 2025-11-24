
#include "DataStoreActor.h"

#include "PersistentDataStore.h"
#if WITH_EDITOR
#include "VisualizerValidator.h"
#endif

#include "StarfirePersistence/Public/PersistenceComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreActor)

ADataStoreActor::ADataStoreActor( )
{
	PersistenceComponent = CreateDefaultSubobject< UPersistenceComponent >( "PersistenceComponent" );
}

void ADataStoreActor::BeginPlay( )
{
	Super::BeginPlay( );

	const auto DataStore = UPersistentDataStore::GetSubsystem( this );
	DataStore->AddDataStoreActor( this, PersistenceComponent->GetGuid( ) );
}

void ADataStoreActor::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	const auto DataStore = UPersistentDataStore::GetSubsystem( this );
	DataStore->RemoveDataStoreActor( this, PersistenceComponent->GetGuid( ) );

	Super::EndPlay( EndPlayReason );
}

#if WITH_EDITOR
EDataValidationResult ADataStoreActor::IsDataValid( class FDataValidationContext &Context ) const
{
	auto Result = Super::IsDataValid( Context );

	if (!UVisualizerValidator::CheckIsValidVisualizerConfig( GetClass( ), Context ))
		return EDataValidationResult::Invalid;

	return Result;
}
#endif
