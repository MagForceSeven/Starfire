
#include "Templates/TypeUtilitiesSF.h"

#include "GameplayTagAssetInterface.h"

namespace TypeUtilitiesSF
{
#if !UE_BUILD_SHIPPING
	void TemplateCompileTests( void )
	{
		// UObjects
		{ const UClass *Class = GetStaticClass< APlayerController >( ); }
		{ const UClass *Class = GetStaticClass< const APlayerController >( ); }

		// TODO: Rider is confused here and reports a concept error, but it compiles & runs fine
		// Interfaces
		{ const UClass *Class = GetStaticClass< IGameplayTagAssetInterface >( ); }
		{ const UClass *Class = GetStaticClass< const IGameplayTagAssetInterface >( ); }

		// Expected errors if uncommented
		//		Non-UObject or Interface types
		//{ const UClass *Class = GetStaticClass< FVector >( ); }
		//{ const UClass *Class = GetStaticClass< const FVector >( ); }
	}
#endif
}