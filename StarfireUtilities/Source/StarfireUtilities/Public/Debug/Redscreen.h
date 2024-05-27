
#pragma once

#include "CoreTypes.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "Templates/IsArrayOrRefOfType.h"

class UObject;

namespace Redscreens
{
	STARFIREUTILITIES_API void Init( const UObject *WorldContext );
}

STARFIREUTILITIES_API void RedscreenImpl( const UObject *WorldContext, bool bOnce, const char *File, int Line, const TCHAR *Fmt, ... );

#define Redscreen( WorldContext, Format, ... ) \
	do { \
	static_assert( TPointerIsConvertibleFromTo< TRemovePointer< decltype(WorldContext) >::Type, const UObject >::Value, "World Context must be a UObject type."); \
	static_assert( TIsArrayOrRefOfType< decltype( Format ), TCHAR >::Value, "Formatting string must be a TCHAR array."); \
	RedscreenImpl( WorldContext, false, __FILE__, __LINE__, Format, ##__VA_ARGS__ ); \
	} while (false)

#define RedscreenOnce( WorldContext, Format, ... ) \
	do { \
	static_assert( TPointerIsConvertibleFromTo< TRemovePointer< decltype(WorldContext) >::Type, const UObject >::Value, "World Context must be a UObject type."); \
	static_assert( TIsArrayOrRefOfType< decltype( Format ), TCHAR >::Value, "Formatting string must be a TCHAR array."); \
	RedscreenImpl( WorldContext, true, __FILE__, __LINE__, Format, ##__VA_ARGS__ ); \
	} while (false)

[[nodiscard]] STARFIREUTILITIES_API FString GetBlueprintCallerFunctionName( void );