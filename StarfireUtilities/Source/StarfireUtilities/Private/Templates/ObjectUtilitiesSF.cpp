
#include "Templates/ObjectUtilitiesSF.h"

namespace ObjectUtilitiesSF
{
#if !UE_BUILD_SHIPPING
	void ForEachTest( APlayerController* ) { }
	void ForEachTest_Const( const APlayerController* ) { }

	void TemplateCompileTests( void )
	{
		//		No deduction possible - exact matches
		{ const TArray< APlayerController* > Instances = GetObjectsOfClass< APlayerController >( ); }
		{ const TArray< const APlayerController* > Instances = GetObjectsOfClass< const APlayerController >( ); }

		//		Deduction possible, but specified - exact matches
		{ TArray< APlayerController* > Instances; GetObjectsOfClass< APlayerController >( Instances ); }
		{ TArray< const APlayerController* > Instances; GetObjectsOfClass< const APlayerController >( Instances ); }

		//		Deduction possible, but specified - safe conversion to const output
		{ TArray< const APlayerController* > Instances; GetObjectsOfClass< APlayerController >( Instances ); }

		//		Types deduced - exact matches
		{ TArray< APlayerController* > Instances; GetObjectsOfClass( Instances ); }
		{ TArray< const APlayerController* > Instances; GetObjectsOfClass( Instances ); }

		//		Deduction not possible - exact matches
		ForEachObjectOfClass< APlayerController >( ForEachTest );
		ForEachObjectOfClass< const APlayerController >( ForEachTest_Const );
		ForEachObjectOfClass< APlayerController >( [ ]( APlayerController* ) -> void { } );
		ForEachObjectOfClass< const APlayerController >( [ ]( const APlayerController* ) -> void { } );

		//		Deduction not possible - safe conversion to const parameter
		ForEachObjectOfClass< APlayerController >( ForEachTest_Const );
		ForEachObjectOfClass< APlayerController >( [ ]( const APlayerController* ) -> void { } );

		//		Types fail deduction - exact matches
		//ForEachObjectOfClass( ForEachTest );
		//ForEachObjectOfClass( ForEachTest_Const );

		//ForEachObjectOfClass( [ ]( APlayerController* ) -> void { } );
		//ForEachObjectOfClass( [ ]( const APlayerController* ) -> void { } );

		//		Type deduced - exact matches
		TFunctionRef< void( APlayerController* ) > ForEachTest_Local( ForEachTest );
		TFunctionRef< void( const APlayerController* ) > ForEachTest_Const_Local( ForEachTest_Const );
		ForEachObjectOfClass( ForEachTest_Local );
		ForEachObjectOfClass( ForEachTest_Const_Local );

		// Expected errors if uncommented
		//		No deduction possible - const/non-const mismatch
		//{ const TArray< const APlayerController* > Instances = GetObjectsOfClass< APlayerController >( ); } // It'd be cool if this one could work though, but that's on TArray
		//{ const TArray< APlayerController* > Instances = GetObjectsOfClass< const APlayerController >( ); }
		//		Discarding result
		//{ GetObjectsOfClass< APlayerController >( ); }
		//{ GetObjectsOfClass< const APlayerController >( ); }
		//		Interfaces Unsupported
		//{ const TArray< IGameplayTagAssetInterface* > Instances = GetObjectsOfClass< IGameplayTagAssetInterface >( ); }
		//{ const TArray< const IGameplayTagAssetInterface* > Instances = GetObjectsOfClass< const IGameplayTagAssetInterface >( ); }
		//		Unsafe conversion from const type to non-const array
		//{ TArray< APlayerController* > Instances; GetObjectsOfClass< const APlayerController >( Instances ); }
		//		Unsafe conversion from const type to non-const parameter
		//ForEachObjectOfClass< const APlayerController >( ForEachTest );
		//ForEachObjectOfClass< const APlayerController >( [ ]( APlayerController* ) -> void { } );
	}
#endif
}