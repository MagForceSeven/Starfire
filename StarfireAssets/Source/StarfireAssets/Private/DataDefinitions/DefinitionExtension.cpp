
#include "DataDefinitions/DefinitionExtension.h"

FPrimaryAssetId UDataDefinitionExtension::GetPrimaryAssetId( ) const
{
	return FPrimaryAssetId( UDataDefinitionExtension::StaticClass( )->GetFName( ), GetFName( ) );
}