
#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "UObject/ObjectMacros.h"

#include "BlueprintUtilitiesSF.generated.h"

// Utility enumeration for use with ExpandEnumAsExecs markup
UENUM( )
enum class ExecEnum_Success : uint8
{
	Success,
	Failure
};

// Utility enumeration for use with ExpandEnumAsExecs markup
UENUM( )
enum class ExecEnum_Validity : uint8
{
	Valid,
	Invalid
};

// Utility enumeration for use with ExpandEnumAsExecs markup
UENUM( )
enum class ExecEnum_YesNo : uint8
{
	Yes,
	No
};

// Utility functions to get around const problems on native-blueprint boundaries when dealing with arrays of pointers
template < class type_t >
UE_NODISCARD TArray< type_t* >& BlueprintCompatibilityCast( TArray< const type_t* > &NativeArray );
template < class type_t >
UE_NODISCARD const TArray< type_t* >& BlueprintCompatibilityCast( const TArray< const type_t* > &NativeArray );

template < class type_t >
UE_NODISCARD TArray< const type_t* >& NativeCompatibilityCast( TArray< type_t* > &BlueprintArray );
template < class type_t >
UE_NODISCARD const TArray< const type_t* >& NativeCompatibilityCast( const TArray< type_t* > &BlueprintArray );

#if CPP
#define SF_BLUEPRINT_UTILITIES_HPP
#include "../../Private/Kismet/BlueprintUtilitiesSF.hpp"
#undef SF_BLUEPRINT_UTILITIES_HPP
#endif