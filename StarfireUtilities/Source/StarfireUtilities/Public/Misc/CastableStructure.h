
#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "UObject/ObjectMacros.h"

#include "CastableStructure.generated.h"

// Utility structure that can be derived from to allow for reflection protected casting
USTRUCT( )
struct STARFIREUTILITIES_API FCastableStructure
{
	GENERATED_BODY( )
public:
	virtual ~FCastableStructure( ) = default;

	// Utility to allow for safe structure casting
	virtual UScriptStruct* GetType( void ) const { return StaticStruct( ); }
};

// Macro to simplify required virtual function overrides by derived types
#define CASTABLE_STRUCTURE_IMPL( ) UScriptStruct* GetType( void ) const override { return StaticStruct( ); }

// Non-const, fail-able casting
template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
[[nodiscard]] dest_type_t* StructCast( src_type_t &Struct );

// Const, fail-able casting
template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
[[nodiscard]] const dest_type_t* StructCast( const src_type_t &Struct );

// Non-const, un-fail-able casting
template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
[[nodiscard]] dest_type_t& StructCastChecked( src_type_t &Struct );

// Const, un-fail-able casting
template < class dest_type_t, class src_type_t >
	requires std::derived_from< dest_type_t, src_type_t >
[[nodiscard]] const dest_type_t& StructCastChecked( const src_type_t &Struct );

#if CPP
#define CASTABLE_STRUCTURE_HPP
#include "../../Private/Misc/CastableStructure.hpp"
#undef CASTABLE_STRUCTURE_HPP
#endif