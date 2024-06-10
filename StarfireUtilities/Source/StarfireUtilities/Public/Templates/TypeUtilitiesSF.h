
#pragma once

#include "Templates/Casts.h"

// PS5 is known to be missing these concepts
// if another platform is missing concepts from it's std implementation,
// either add another platform case or add "STARFIRE_PLATFORM_MISSING_CONCEPTS=1" as a project definition in your Target.cs

#if defined(PLATFORM_PS5) && PLATFORM_PS5
	#define SF_CUSTOM_CONCEPTS 1
#elif defined (STARFIRE_PLATFORM_MISSING_CONCEPTS) && STARFIRE_PLATFORM_MISSING_CONCEPTS
	#define SF_CUSTOM_CONCEPTS 1
#else
	#define SF_CUSTOM_CONCEPTS 0
#endif

// Work around missing header/implementations on some platforms
namespace SFstd
{
#if SF_CUSTOM_CONCEPTS
	// Copied from Epic's TypedElementQueryBuilder.inl source for implementations
	namespace detail
	{
		template< class type1_t, class type2_t >
		concept SameHelper = std::is_same_v< type1_t, type2_t >;
	}
	template< class type1_t, class type2_t >
	concept same_as = detail::SameHelper< type1_t, type2_t > && detail::SameHelper< type2_t, type1_t >;

	template < class from_t, class to_t >
	concept convertible_to = std::is_convertible_v< from_t, to_t > && requires { static_cast< to_t >( std::declval< from_t >( ) ); };

	template< class derived_t, class base_t >
	concept derived_from = std::is_base_of_v< base_t, derived_t > && std::is_convertible_v< const volatile derived_t*, const volatile base_t* >;
#else
	// Our version still uses STL on the platforms that do support them
	template < class type1_t, class type2_t >
	concept same_as = std::same_as< type1_t, type2_t >;

	template < class from_t, class to_t >
	concept convertible_to = std::convertible_to< from_t, to_t >;

	template < class derived_t, class base_t >
	concept derived_from = std::derived_from< derived_t, base_t >;
#endif

	// A few concepts that aren't anywhere in std:: but feel enough like standard library checks that they should share
	// this location/style vs Epic style

	template < class type_t >
	concept void_pointer_compatible = requires { static_cast< void* >( std::declval< type_t >() ); };

	template < class type_t >
	concept is_const_pointer = std::is_pointer_v< type_t > && !void_pointer_compatible< type_t >;
	
	template < class type_t >
	concept is_mutable_pointer = std::is_pointer_v< type_t > && void_pointer_compatible< type_t >;
}

// Utility concept for restricting template parameters
template < class type_t >
concept CInterfaceType = !!TIsIInterface< type_t >::Value;
template < class type_t >
concept CObjectType = SFstd::derived_from< type_t, UObject >;
template < class type_t >
concept CActorType = SFstd::derived_from< type_t, AActor >;

// Template utility for properly getting the the UClass for a type declaration
// Copied from some other UE4 utility
template < CInterfaceType type_t >
UClass* GetStaticClass( );

template < CObjectType type_t >
UClass* GetStaticClass( );

// Utility function for checking if an object is a certain type (taking the difference in interface checks into account)
[[nodiscard]] FORCEINLINE bool TypeMatch( const UObject *Object, const UClass *Type );

#if CPP
#define TYPE_UTILITIES_SF_HPP
#include "../../Private/Templates/TypeUtilitiesSF.hpp"
#undef TYPE_UTILITIES_SF_HPP
#endif
