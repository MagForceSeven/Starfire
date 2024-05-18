
#pragma once

#include "UObject/Interface.h"

#include "VerifiableContent.generated.h"

class FKismetCompilerContext;

class IVerifiableAsset;
class IVerifiableStrategy;

// Utilities for restricting template parameters to asset and strategy implementors
template < class type_t >
concept VerifiableAsset = std::derived_from< type_t, IVerifiableAsset >;
template < class type_t >
concept VerifiableStrategy = std::derived_from< type_t, IVerifiableStrategy >;

// Static class for assets which support verifying their data with Asset Check calls
UINTERFACE( meta = (CannotImplementInterfaceInBlueprint) )
class STARFIREASSETS_API UVerifiableAsset : public UInterface
{
	GENERATED_BODY( )
public:
};

// Interface class for assets which support verifying their data with Asset Check calls
class STARFIREASSETS_API IVerifiableAsset
{
	GENERATED_BODY( )
public:

	// A hook for derived asset types to check their configuration state
	virtual void Verify( const UObject *WorldContext ) { }

	// A utility for handling the verification of an asset and get around const-ness issues
	template < VerifiableAsset type_t >
	static void Verify( const type_t* Asset, const UObject *WorldContext );

	// A helper utility for calling Verify on a collection of verifiable assets
	template < VerifiableAsset type_t >
	static void VerifyAll( const TArray< const type_t* > &Assets, const UObject *WorldContext );

	// A helper utility for calling Verify on a collection of verifiable assets
	template < VerifiableAsset type_t >
	static void VerifyAll( const TArray< type_t* > &Assets, const UObject *WorldContext );

	// Determine if there are nullptrs in the array and remove them (when there's a valid world context)
	template < class type_t >
	UE_NODISCARD static bool SanitizeArray( TArray< type_t* > &Array, const UObject *WorldContext );
};

// Static class for instanced sub-objects which support verifying their data with Asset Check calls
UINTERFACE( meta = (CannotImplementInterfaceInBlueprint) )
class STARFIREASSETS_API UVerifiableStrategy : public UInterface
{
	GENERATED_BODY( )
public:
};

// Interface class for instanced sub-objects which support verifying their data with Asset Check calls
class STARFIREASSETS_API IVerifiableStrategy
{
	GENERATED_BODY( )
public:
	// A hook for derived instanced objects to check their configuration state
	virtual void Verify( const UObject *Asset, const UObject *WorldContext ) { }
	virtual bool VerifyEx( const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext ) { return true; }

	// A utility for handling the verification of a sub-object and get around const-ness issues
	template < VerifiableStrategy type_t >
	static bool Verify( const type_t *SubObject, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext = nullptr );

	// A helper utility for calling Verify on a collection of verifiable sub-objects
	template < VerifiableStrategy type_t >
	static bool VerifyAll( const TArray< const type_t* > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext = nullptr );
	template < VerifiableStrategy type_t >
	static bool VerifyAll( const TArray< type_t* > &SubObjects, const UObject *Asset, const UObject *WorldContext, FKismetCompilerContext *CompilerContext = nullptr );
};

// Remove nullptrs from the array (when the WorldContext is valid) and verify all the remaining elements of the array
#define VERIFY_AND_SANITIZE( Array, Asset, WorldContext ) \
if (IVerifiableAsset::SanitizeArray( Array, WorldContext )) \
	AssetChecks::AC_Message( Asset, TEXT( #Array " configured with one or more nullptr instances." ), WorldContext ); \
IVerifiableStrategy::VerifyAll( Array, Asset, WorldContext );

// Remove nullptrs from the array (when the WorldContext is valid) and verify all the remaining elements of the array
#define VERIFY_AND_SANITIZE_EX( Array, Asset, WorldContext, Context )																\
[ this, Asset, WorldContext, Context ]( auto &A ) -> bool																			\
{																																	\
	bool Verified = true;																											\
	if (IVerifiableAsset::SanitizeArray( A, WorldContext ))																			\
	{																																\
		AssetChecks::AC_Message( Asset, TEXT( #Array " configured with one or more nullptr instances." ), this, WorldContext );	\
		Verified = false;																											\
	}																																\
																																	\
	return IVerifiableStrategy::VerifyAll( A, Asset, WorldContext, Context ) && Verified;											\
}( Array );

#if CPP
#define VERIFIABLE_CONTENT_HPP
#include "../../Private/AssetValidation/VerifiableContent.hpp"
#undef VERIFIABLE_CONTENT_HPP
#endif