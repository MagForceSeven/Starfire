
#pragma once

#include "UObject/Interface.h"

#include "DataDefinitionSource.generated.h"

class UDataDefinition;

// Static class for interfaces meant to be implemented by objects that represent specific data definition instances
UINTERFACE( BlueprintType )
class STARFIREASSETS_API UDataDefinitionSource : public UInterface
{
	GENERATED_BODY( )
public:
};

// Actual interface class to be implemented by objects that represent specific data definition instances
class STARFIREASSETS_API IDataDefinitionSource
{
	GENERATED_BODY( )
public:
	// Hook for getting the definition most strongly related to this object
	UFUNCTION( BlueprintNativeEvent )
	UE_NODISCARD const UDataDefinition* GetSourceDefinition( ) const;
	virtual const UDataDefinition* GetSourceDefinition_Implementation( ) const PURE_VIRTUAL( GetSourceDefinition_Implementation, return nullptr; );
};
