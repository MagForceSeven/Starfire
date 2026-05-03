
#pragma once

#include "UObject/Interface.h"

#include "UI/TextInjection/TextInjectionShared.h"

#include "Templates/ArrayTypeUtilitiesSF.h"

#include "TextTagResolver.generated.h"

// The collection of Tags that should be replaced with contextual
struct STARFIREUI_API FTextTags
{
	// Valid Constructors
	FTextTags( TMap< FString, FText > &TagMap );
	FTextTags( const FString &Scope, const FTextTags &Tags );

	// See if a specific tag exists in the map and needs to be filled in
	FText* FindTag( const FString &Tag ) const;

	// Assign some text to a tag if it exists in the map
	void AssignToTag( const FString &Tag, const FText &Text ) const;

private:
	// Additional scope to narrow tag searches
	FString Scope;
	// Collection of tags that resolver should attempt to replace
	TMap< FString, FText > &Inner;
};

// A single record of an object that should be used to resolve tags in text
struct STARFIREUI_API FTagResolver
{
	// Constructors
	FTagResolver( ) = default;
	FTagResolver( const UObject *R, const FTextTagContexts &C, const FString &S = { } );

	// The object that should be used to resolve tags
	const UObject *Resolver = nullptr;

	// The contextual data that this resolver should use when resolving tags
	FTextTagContexts Contexts;

	// The scope that should be used when resolving tags for this resolver
	FString Scope;
};

// Static class for interfaces meant to be implemented by data definitions to perform text injection
UINTERFACE( MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint) )
class UTextTagResolver : public UInterface
{
	GENERATED_BODY( )
public:
};

// Actual interface class to be implemented by data definitions to perform text injection
class STARFIREUI_API ITextTagResolver
{
	GENERATED_BODY( )
public:
	// Hook for derived types to adjust the initial set of contexts before the resolution process begins
	virtual void PreResolveUpdateContexts( FTextTagContexts &Contexts, const UObject *WorldContext ) const { }

	// Hook for derived types to add more objects to the collection of objects that should be used to resolve the tags
	virtual void AppendAdditionalResolvers( TArray< FTagResolver > &outResolvers, const FTextTagContexts &Contexts, const UObject *WorldContext ) const { }

	// API to convert a collection of tags from static text with values from runtime data
	virtual void ResolveTextTags( const FTextTags &Tags, const FTextTagContexts &Contexts, const UObject *WorldContext ) const;

protected:
	// Utilities to expand a collection of objects into new FTagResolver entries all with the same context and scope
	template < CObjectType type_t >
	static void AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< const type_t* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope = { } );
	template < CObjectType type_t >
	static void AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< type_t* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope = { } );
	static void AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< const UObject* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope = { } );
};

template < CObjectType type_t >
void ITextTagResolver::AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< const type_t* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope )
{
	auto &CastArray = ArrayUpCast< UObject >( Resolvers );
	AppendResolvers( outResolvers, CastArray, Contexts, Scope );
}

template < CObjectType type_t >
void ITextTagResolver::AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< type_t* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope )
{
	auto &CastArray = ArrayUpCast< const UObject >( Resolvers );
	AppendResolvers( outResolvers, CastArray, Contexts, Scope );
}