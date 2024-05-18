
#pragma once

// A structure for invoking a lambda through an operator overload which allows the scope to come after the invocation
struct FInvokedScopeCaller
{
	template< typename type_t >
	FORCEINLINE auto operator+( type_t&& Lambda ) -> decltype( auto )
	{
		return Lambda( );
	}
};

// Macro that can create and invoke a lambda but make it look like a regular non-lambda scope
//
// var = INVOKED_SCOPE
// {
//
//	  .. complicate logic
//
//    return something
// };
#define INVOKED_SCOPE FInvokedScopeCaller( ) + [ & ]( )