
#include "DataDefinitions/DefinitionExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DefinitionExtension)

FPrimaryAssetId UDataDefinitionExtension::GetPrimaryAssetId( ) const
{
	return FPrimaryAssetId( UDataDefinitionExtension::StaticClass( )->GetFName( ), GetFName( ) );
}