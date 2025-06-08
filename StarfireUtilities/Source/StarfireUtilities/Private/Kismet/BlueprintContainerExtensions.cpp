
#include "Kismet/BlueprintContainerExtensions.h"

#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/BlueprintSetLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintContainerExtensions)

DEFINE_FUNCTION( UBlueprintContainerExtensions::execSet_Get )
{
	UBlueprintSetLibrary::execSet_GetItemByIndex( Context, Stack, Z_Param__Result );
}

void UBlueprintContainerExtensions::GenericSet_Get( void *TargetSet, const FSetProperty *SetProperty, int32 Index, void *Item )
{
	UBlueprintSetLibrary::GenericSet_GetItemByIndex( TargetSet, SetProperty, Index, Item );
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execSet_LastIndex )
{
	UBlueprintSetLibrary::execSet_GetLastIndex( Context, Stack, Z_Param__Result );
}

int32 UBlueprintContainerExtensions::GenericSet_LastIndex( const void* TargetSet, const FSetProperty* SetProperty )
{
	return UBlueprintSetLibrary::GenericSet_GetLastIndex( TargetSet, SetProperty );
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execMap_Get )
{
	UBlueprintMapLibrary::execMap_GetKeyValueByIndex( Context, Stack, Z_Param__Result );
}

void UBlueprintContainerExtensions::GenericMap_Get( const void *TargetMap, const FMapProperty *MapProperty, int32 Index, void *Key, void *Item )
{
	UBlueprintMapLibrary::GenericMap_GetKeyValueByIndex( TargetMap, MapProperty, Index, Key, Item );
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execMap_LastIndex )
{
	UBlueprintMapLibrary::execMap_GetLastIndex( Context, Stack, Z_Param__Result );
}

int32 UBlueprintContainerExtensions::GenericMap_LastIndex( const void* TargetMap, const FMapProperty* MapProperty )
{
	return UBlueprintMapLibrary::GenericMap_GetLastIndex( TargetMap, MapProperty );
}