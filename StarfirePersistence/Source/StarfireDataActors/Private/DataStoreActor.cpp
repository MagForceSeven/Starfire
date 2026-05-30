
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
	PersistenceComponent->SetUseSaveGameMeta( false );
}

void ADataStoreActor::PostRegisterAllComponents( )
{
	Super::PostRegisterAllComponents( );

	if (const auto DataStore = UPersistentDataStore::GetSubsystem( this ))
		DataStore->AddDataStoreActor( this, PersistenceComponent->GetGuid( ) );
}

void ADataStoreActor::BeginPlay( )
{
	Super::BeginPlay( );

	// Update folder and actor label on Begin Play so that Actor that affects label can be configured during deferred spawning
#if WITH_EDITOR
	const auto Label = GetCustomActorLabel( );
	if (!Label.IsEmpty( ))
		SetActorLabel( Label );

	auto CustomFolderPath = GetCustomOutlinerFolder( );
	if (CustomFolderPath.IsEmpty( ))
		CustomFolderPath = "Data Store";
	else
		CustomFolderPath = "Data Store/" + CustomFolderPath;

	SetFolderPath( FName( CustomFolderPath ) );
#endif
}

void ADataStoreActor::Destroyed( )
{
	if (const auto DataStore = UPersistentDataStore::GetSubsystem( this ))
		DataStore->RemoveDataStoreActor( this, PersistenceComponent->GetGuid( ) );

	Super::Destroyed( );
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
