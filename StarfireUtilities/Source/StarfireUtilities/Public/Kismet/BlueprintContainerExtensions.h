
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BlueprintContainerExtensions.generated.h"

// Functions that should be part of the BlueprintSetLibrary or BlueprintMapLibrary
// Set function implementations from https://github.com/EpicGames/UnrealEngine/pull/11622
// Map functions also included in PR https://github.com/EpicGames/UnrealEngine/pull/10355
UCLASS( )
class UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintMapLibrary or BlueprintSetLibrary instead.")
	STARFIREUTILITIES_API UBlueprintContainerExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:

	/**
	 * Given a set and an index, returns a copy of the item found at that index.
	 * Index may differ from the order in which they were added. This function is implemented
	 * for purpose of iterating Sets by index.
	 *
	 * @param	TargetSet		The set to get an item from
	 * @param	Index			The index in the set to get an item from
	 * @param	Item			A copy of the item stored at the index
	*/
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintSetLibrary instead.")
	UFUNCTION( BlueprintPure, CustomThunk, meta = (DeprecatedFunction, DisplayName = "Get", CompactNodeTitle = "GET", SetParam = "TargetSet|Item", AutoCreateRefTerm = "Item", BlueprintThreadSafe), Category = "Utilities|Set" )
	static void Set_Get( const TSet< int32 > &TargetSet, int32 Index, int32 &Item );

	DECLARE_FUNCTION( execSet_Get );

	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintSetLibrary instead.")
	static void GenericSet_Get( void *TargetSet, const FSetProperty *SetProperty, int32 Index, void *Item );

	/** 
	 * Determines the last valid index for a set
	 *
	 * @param	TargetSet		The set in question
	 * @return	The number of entries in the set
	 */
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintSetLibrary instead.")
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DeprecatedFunction, DisplayName = "Last Index", CompactNodeTitle = "LAST INDEX", SetParam = "TargetSet", BlueprintThreadSafe), Category="Utilities|Set")
	static int32 Set_LastIndex( const TSet< int32 > &TargetSet );

	DECLARE_FUNCTION(execSet_LastIndex);
	
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintSetLibrary instead.")
	static int32 GenericSet_LastIndex( const void* TargetSet, const FSetProperty* SetProperty );
	
	/**
	 * Given a map and an index, returns a copy of the key and value found at that index.
	 * Index may differ from the order in which they were added. This function is implemented
	 * for purpose of iterating Maps by index.
	 *
	 * @param	TargetMap	The map to get a key & value from
	 * @param	Index		The index in the map to get the key & value
	 * @param	Key			A copy of the key stored in the map at the index
	 * @param	Value		A copy of the value stored in the map at the index
	 */
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintMapLibrary instead.")
	UFUNCTION( BlueprintPure, CustomThunk, meta =(DeprecatedFunction, DisplayName = "Get", CompactNodeTitle = "GET", MapParam = "TargetMap", MapKeyParam = "Key", MapValueParam = "Value", AutoCreateRefTerm = "Key, Value", BlueprintThreadSafe), Category = "Utilities|Map" )
	static void Map_Get( const TMap< int32, int32 > &TargetMap, int32 Index, int32 &Key, int32 &Value );

	DECLARE_FUNCTION( execMap_Get );

	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintMapLibrary instead.")
	static void GenericMap_Get( const void *TargetMap, const FMapProperty *MapProperty, int32 Index, void *Key, void *Item );

	/** 
	 * Determines the last valid index for a map
	 *
	 * @param	TargetMap		The map in question
	 * @return	The number of entries in the map
	 */
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintMapLibrary instead.")
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DeprecatedFunction, DisplayName = "Last Index", CompactNodeTitle = "LAST INDEX", MapParam = "TargetMap", BlueprintThreadSafe), Category="Utilities|Map")
	static int32 Map_LastIndex( const TMap< int32, int32 > &TargetMap );

	DECLARE_FUNCTION(execMap_LastIndex);
	
	UE_DEPRECATED(5.6, "Container extensions are no longer necessary. Use BlueprintMapLibrary instead.")
	static int32 GenericMap_LastIndex( const void* TargetMap, const FMapProperty* MapProperty );
};