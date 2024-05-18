
#pragma once

#include "Templates/TypeUtilitiesSF.h"

//**********************************************************************************************************************
// Utilities for casting arrays of uobject pointers (and soft pointers) from a child type to a parent type

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD TArray< type_t* >& ArrayUpCast( TArray< source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD const TArray< type_t* >& ArrayUpCast( const TArray< source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD TArray< const type_t* >& ArrayUpCast( TArray< const source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD const TArray< const type_t* >& ArrayUpCast( const TArray< const source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD TArray< TSoftObjectPtr< type_t > >& ArrayUpCast( TArray< TSoftObjectPtr< source_t > > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< source_t, type_t >
UE_NODISCARD const TArray< TSoftObjectPtr< type_t > >& ArrayUpCast( const TArray< TSoftObjectPtr< source_t > > &Source );

//**********************************************************************************************************************
// Utilities for casting arrays of uobject pointers (and soft pointers) from a parent type to a child type
// Caller is responsible for guaranteeing that all pointers are actually the child type, otherwise access violations will occur

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD TArray< type_t* >& ArrayDownCast( TArray< source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD const TArray< type_t* >& ArrayDownCast( const TArray< source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD TArray< const type_t* >& ArrayDownCast( TArray< const source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD const TArray< const type_t* >& ArrayDownCast( const TArray< const source_t* > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD TArray< TSoftObjectPtr< type_t > >& ArrayDownCast( TArray< TSoftObjectPtr< source_t > > &Source );

template < class type_t, class source_t >
	requires SFstd::derived_from< type_t, source_t >
UE_NODISCARD const TArray< TSoftObjectPtr< type_t > >& ArrayDownCast( const TArray< TSoftObjectPtr< source_t > > &Source );

//**********************************************************************************************************************
// Utilities for casting arrays of UObjects into arrays of interfaces

template < CInterfaceType interface_t, CObjectType source_t >
UE_NODISCARD TArray< interface_t* > InterfaceArrayCastChecked( const TArray< source_t* > &Source );

template < CInterfaceType interface_t, CObjectType source_t >
UE_NODISCARD TArray< const interface_t* > InterfaceArrayCastChecked( const TArray< const source_t* > &Source );

enum class EInterfaceCastMode
{
	ExcludeNulls,
	IncludeNulls
};

template < CInterfaceType interface_t, CObjectType source_t >
UE_NODISCARD TArray< interface_t* > InterfaceArrayCast( const TArray< source_t* > &Source, EInterfaceCastMode CastMode = EInterfaceCastMode::ExcludeNulls );

template < CInterfaceType interface_t, CObjectType source_t >
UE_NODISCARD TArray< const interface_t* > InterfaceArrayCast( const TArray< const source_t* > &Source, EInterfaceCastMode CastMode = EInterfaceCastMode::ExcludeNulls );

#if CPP
#define ARRAY_TYPE_UTILITIES_SF_HPP
#include "../../Private/Templates/ArrayTypeUtilitiesSF.hpp"
#undef CORE_TYPE_UTILITIES_HPP
#endif