
#pragma once

#include "DataStoreActor.h"

#include "DataStoreSingleton.generated.h"

// A data store type that should only have a single instance
UCLASS( Abstract )
class STARFIREDATAACTORS_API ADataStoreSingleton : public ADataStoreActor
{
	GENERATED_BODY( )
public:
	// Actor API
	void PostActorCreated( ) override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Data Store Actor API
#if WITH_EDITOR
	FString GetCustomActorLabel_Implementation( ) const override;
	FString GetCustomOutlinerFolder_Implementation( ) const override;
#endif
};