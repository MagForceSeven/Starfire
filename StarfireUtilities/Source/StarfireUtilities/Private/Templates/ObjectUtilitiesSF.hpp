
// ReSharper disable once CppMissingIncludeGuard
#ifndef OBJECT_UTILITIES_SF_HPP
	#error You shouldn't be including this file directly
#endif

#include "Kismet/BlueprintUtilitiesSF.h"
#include "Templates/ArrayTypeUtilitiesSF.h"

namespace ObjectUtilitiesSF
{
	template < CObjectType type_t >
	TArray< type_t* > GetObjectsOfClass( bool bIncludeDerivedClasses, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusionInternalFlags )
	{
		TArray< type_t* > Results;

		// call our own templated utility which will also handle any const compatibility
		GetObjectsOfClass< type_t >( Results, bIncludeDerivedClasses, ExcludeFlags, ExclusionInternalFlags );

		return MoveTemp( Results );
	}

	template < CObjectType type_t >
	void GetObjectsOfClass( TArray< type_t* > &Results, bool bIncludeDerivedClasses, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusionInternalFlags )
	{
		if constexpr (std::is_const_v< type_t >)
			// call the is_mutable_pointer version to handle array template parameter compatibility
			GetObjectsOfClass< std::remove_cv_t< type_t > >( Results, bIncludeDerivedClasses, ExcludeFlags, ExclusionInternalFlags );
		else
			::GetObjectsOfClass( type_t::StaticClass( ), ArrayUpCast< UObject >( Results ), bIncludeDerivedClasses, ExcludeFlags, ExclusionInternalFlags );
	}

	template < CObjectType type_t >
		requires SFstd::is_mutable_pointer< type_t* >
	void GetObjectsOfClass( TArray< const type_t* > &Results, bool bIncludeDerivedClasses, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusionInternalFlags )
	{
		auto &ObjectType = ArrayUpCast< const UObject >( Results );	// convert to a UObject* for the Engine call
		auto &MutableType = BlueprintCompatibilityCast( ObjectType );	// use BlueprintCompatibilityCast as a const-cast
		
		::GetObjectsOfClass( type_t::StaticClass( ), MutableType, bIncludeDerivedClasses, ExcludeFlags, ExclusionInternalFlags );
	}

	template < CObjectType type_t >
	void ForEachObjectOfClass( TFunctionRef< void( type_t* ) > Operation, bool bIncludeDerivedClasses, EObjectFlags ExcludeFlags, EInternalObjectFlags ExclusionInternalFlags )
	{
		const auto ObjectOperation = [ Operation ]( UObject* Object ) -> void
		{
			type_t* CastObject = CastChecked< type_t >( Object );
			Operation( CastObject );
		};
		
		::ForEachObjectOfClass( type_t::StaticClass( ), ObjectOperation, bIncludeDerivedClasses, ExcludeFlags, ExclusionInternalFlags );
	}
}

/*
From Northstar - possible partial solution to make "ForEachObjectOfClass( Callback )" deduce the type being iterated
template <typename T>
struct TCallableTraits;

template <typename C, CIsDataStreamTopic TopicT>
struct TCallableTraits<void (C::*)(const TopicT&, const FCyDataStreamContext&)>
{
using Type = TopicT;
};

template <typename C, CIsDataStreamTopic TopicT>
struct TCallableTraits<void (C::*)(const TopicT&, const FCyDataStreamContext&) const>
{
using Type = TopicT;
};
*/