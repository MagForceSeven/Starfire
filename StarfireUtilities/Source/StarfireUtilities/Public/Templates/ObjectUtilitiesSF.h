
#pragma once

#include "Templates/TypeUtilitiesSF.h"

// Template versions of similarly named functions in UObjectHash.h
namespace ObjectUtilitiesSF
{
	// Returns an array of objects of a specific class. Optionally, results can include objects of derived classes as well (default = true)
	//	Template parameter must always be specified as no type deduction can be done from the return value type
	//	const elements can be returned by including const in the template parameter: GetObjectsOfClass< const AActor >( );
	template < CObjectType type_t >
	[[nodiscard]] TArray< type_t* > GetObjectsOfClass( bool bIncludeDerivedClasses = true, EObjectFlags ExcludeFlags = RF_ClassDefaultObject, EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::None );

	// Returns an array of objects of a specific class. Optionally, results can include objects of derived classes as well (default = true)
	//	Type can be deduced based on the input TArray or specified explicitly
	template < CObjectType type_t >
	void GetObjectsOfClass( TArray< type_t* > &Results, bool bIncludeDerivedClasses = true, EObjectFlags ExcludeFlags = RF_ClassDefaultObject, EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::None );

	// Returns an array of objects of a specific class. Optionally, results can include objects of derived classes as well (default = true)
	//	Type can be deduced based on the input TArray or specified explicitly
	template < CObjectType type_t >
		requires SFstd::is_mutable_pointer< type_t* >
	void GetObjectsOfClass( TArray< const type_t* > &Results, bool bIncludeDerivedClasses = true, EObjectFlags ExcludeFlags = RF_ClassDefaultObject, EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::None );

	// Performs an operation on all objects of the provided class
	//	Note that the operation must not modify UObject hash maps so it can not create, rename or destroy UObjects.
	//	Template parameter should always be specified for greatest flexibility of operation inputs
	template < CObjectType type_t >
	void ForEachObjectOfClass( TFunctionRef< void( type_t* ) > Operation, bool bIncludeDerivedClasses = true, EObjectFlags ExcludeFlags = RF_ClassDefaultObject, EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::None );
}

#if CPP
#define OBJECT_UTILITIES_SF_HPP
#include "../../Private/Templates/ObjectUtilitiesSF.hpp"
#undef OBJECT_UTILITIES_SF_HPP
#endif
