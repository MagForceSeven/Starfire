
#include "Kismet/BlueprintContainerExtensions.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintContainerExtensions)

DEFINE_FUNCTION( UBlueprintContainerExtensions::execSet_Get )
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn< FSetProperty >( nullptr );
	void *SetAddress = Stack.MostRecentPropertyAddress;
	FSetProperty *SetProperty = CastField< FSetProperty >( Stack.MostRecentProperty );
	if (SetProperty == nullptr)
	{
		Stack.bArrayContextFailed = true;
		return;
	}

	P_GET_PROPERTY( FIntProperty, Index );

	// Since Item isn't really an int, step the stack manually
	const FProperty *ElementProp = SetProperty->ElementProp;
	const int32 PropertySize = ElementProp->GetElementSize( ) * ElementProp->ArrayDim;
	void *StorageSpace = FMemory_Alloca( PropertySize );
	ElementProp->InitializeValue( StorageSpace );

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn< FProperty >( StorageSpace );
	const FFieldClass *ElementPropClass = ElementProp->GetClass( );
	const FFieldClass *MostRecentPropClass = Stack.MostRecentProperty->GetClass( );

	// If the destination and the inner type are identical in size and their field classes derive from one another, then permit the writing out of the array element to the destination memory
	void *ItemPtr = StorageSpace;
	if (Stack.MostRecentPropertyAddress != nullptr && (PropertySize == Stack.MostRecentProperty->GetElementSize( ) * Stack.MostRecentProperty->ArrayDim) &&
		(MostRecentPropClass->IsChildOf(ElementPropClass) || ElementPropClass->IsChildOf(MostRecentPropClass)))
	{
		ItemPtr = Stack.MostRecentPropertyAddress;
	}

	P_FINISH;

	P_NATIVE_BEGIN;
		GenericSet_Get( SetAddress, SetProperty, Index, ItemPtr );
	P_NATIVE_END;

	ElementProp->DestroyValue( StorageSpace );
}

void UBlueprintContainerExtensions::GenericSet_Get( void *TargetSet, const FSetProperty *SetProperty, int32 Index, void *Item )
{
	if (TargetSet)
	{
		FScriptSetHelper SetHelper( SetProperty, TargetSet );

		FProperty *ElementProp = SetProperty->ElementProp;
		if (SetHelper.IsValidIndex(Index))
		{
			ElementProp->CopySingleValueToScriptVM( Item, SetHelper.GetElementPtr( Index ) );
		}
		else
		{
			FFrame::KismetExecutionMessage( *FString::Printf( TEXT("Attempted to access index %d from set '%s' of length %d in '%s'!"),
				Index,
				*SetProperty->GetName( ),
				SetHelper.Num( ),
				*SetProperty->GetOwnerVariant( ).GetPathName( )),
				ELogVerbosity::Warning,
				FName("GetOutOfBoundsWarning"));

			ElementProp->InitializeValue( Item );
		}
	}
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execSet_LastIndex )
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn< FSetProperty >( nullptr );
	void *SetAddress = Stack.MostRecentPropertyAddress;
	FSetProperty* SetProperty = CastField< FSetProperty >( Stack.MostRecentProperty );
	if (SetProperty == nullptr)
	{
		Stack.bArrayContextFailed = true;
		return;
	}
	
	P_FINISH;

	P_NATIVE_BEGIN;
		*(int32*)RESULT_PARAM = GenericSet_LastIndex( SetAddress, SetProperty );
	P_NATIVE_END
}

int32 UBlueprintContainerExtensions::GenericSet_LastIndex( const void* TargetSet, const FSetProperty* SetProperty )
{
	if (TargetSet != nullptr)
	{
		FScriptSetHelper SetHelper( SetProperty, TargetSet );

		return SetHelper.Num( ) - 1;
	}

	return 0;
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execMap_Get )
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn< FMapProperty >( nullptr );
	void *MapAddress = Stack.MostRecentPropertyAddress;
	FMapProperty *MapProperty = CastField< FMapProperty >( Stack.MostRecentProperty );
	if (MapProperty == nullptr)
	{
		Stack.bArrayContextFailed = true;
		return;
	}
	
	P_GET_PROPERTY( FIntProperty, Index );
	
	// Since Key isn't really an int, step the stack manually
	const FProperty *CurrKeyProp = MapProperty->KeyProp;
	const int32 KeyPropertySize = CurrKeyProp->GetElementSize( ) * CurrKeyProp->ArrayDim;
	void* KeyStorageSpace = FMemory_Alloca( KeyPropertySize );
	CurrKeyProp->InitializeValue( KeyStorageSpace );

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn< FProperty >( KeyStorageSpace );
	const FFieldClass *CurrKeyPropClass = CurrKeyProp->GetClass( );
	const FFieldClass *MostRecentPropClass = Stack.MostRecentProperty->GetClass( );

	// If the destination and the inner type are identical in size and their field classes derive from one another, then permit the writing out of the array element to the destination memory
	void *KeyPtr = KeyStorageSpace;
	if (Stack.MostRecentPropertyAddress != nullptr && (KeyPropertySize == Stack.MostRecentProperty->GetElementSize( ) * Stack.MostRecentProperty->ArrayDim) &&
		(MostRecentPropClass->IsChildOf( CurrKeyPropClass ) || CurrKeyPropClass->IsChildOf( MostRecentPropClass )))
	{
		KeyPtr = Stack.MostRecentPropertyAddress;
	}

	// Since the Value isn't really an int, step the stack manually
	const FProperty *CurrValueProp = MapProperty->ValueProp;
	const int32 ValuePropertySize = CurrValueProp->GetElementSize( ) * CurrValueProp->ArrayDim;
	void *ValueStorageSpace = FMemory_Alloca( ValuePropertySize );
	CurrValueProp->InitializeValue( ValueStorageSpace );
	
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn< FProperty >( ValueStorageSpace );
	const FFieldClass *CurrValuePropClass = CurrValueProp->GetClass( );
	MostRecentPropClass = Stack.MostRecentProperty->GetClass( );

	// If the destination and the inner type are identical in size and their field classes derive from one another, then permit the writing out of the array element to the destination memory
	void *ItemPtr = ValueStorageSpace;
	if (Stack.MostRecentPropertyAddress != nullptr && (ValuePropertySize == Stack.MostRecentProperty->GetElementSize( ) * Stack.MostRecentProperty->ArrayDim) &&
		(MostRecentPropClass->IsChildOf( CurrValuePropClass ) || CurrValuePropClass->IsChildOf( MostRecentPropClass )))
	{
		ItemPtr = Stack.MostRecentPropertyAddress;
	}
		
	P_FINISH;

	P_NATIVE_BEGIN;
		GenericMap_Get( MapAddress, MapProperty, Index, KeyPtr, ItemPtr );
	P_NATIVE_END;

	CurrValueProp->DestroyValue( ValueStorageSpace );
	CurrKeyProp->DestroyValue( KeyStorageSpace );

}

void UBlueprintContainerExtensions::GenericMap_Get( void *TargetMap, const FMapProperty *MapProperty, int32 Index, void *Key, void *Item )
{
	if (TargetMap != nullptr)
	{
		FScriptMapHelper MapHelper( MapProperty, TargetMap );
		
		FProperty *KeyProp = MapProperty->KeyProp;
		FProperty *ValueProp = MapProperty->ValueProp;

		const auto Iterator = MapHelper.CreateIterator( Index );
		
		if (MapHelper.IsValidIndex( Iterator.GetInternalIndex( ) ))
		{
			KeyProp->CopySingleValueToScriptVM( Key, MapHelper.GetKeyPtr( Iterator.GetInternalIndex( ) ) );
			ValueProp->CopySingleValueToScriptVM( Item, MapHelper.GetValuePtr( Iterator.GetInternalIndex( ) ) );
		}
		else
		{
			FFrame::KismetExecutionMessage( *FString::Printf( TEXT("Attempted to access index %d from set '%s' of length %d in '%s'!"),
				Index,
				*MapProperty->GetName( ),
				MapHelper.Num( ),
				*MapProperty->GetOwnerVariant( ).GetPathName( )),
				ELogVerbosity::Warning,
				FName("GetOutOfBoundsWarning"));
		}
	}
}

DEFINE_FUNCTION( UBlueprintContainerExtensions::execMap_LastIndex )
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn< FMapProperty >( nullptr );
	void *MapAddress = Stack.MostRecentPropertyAddress;
	FMapProperty* MapProperty = CastField< FMapProperty >( Stack.MostRecentProperty );
	if (MapProperty == nullptr)
	{
		Stack.bArrayContextFailed = true;
		return;
	}
	
	P_FINISH;

	P_NATIVE_BEGIN;
		*(int32*)RESULT_PARAM = GenericMap_LastIndex( MapAddress, MapProperty );
	P_NATIVE_END
}

int32 UBlueprintContainerExtensions::GenericMap_LastIndex( const void* TargetMap, const FMapProperty* MapProperty )
{
	if (TargetMap != nullptr)
	{
		FScriptMapHelper MapHelper( MapProperty, TargetMap );

		return MapHelper.Num( ) - 1;
	}

	return 0;
}