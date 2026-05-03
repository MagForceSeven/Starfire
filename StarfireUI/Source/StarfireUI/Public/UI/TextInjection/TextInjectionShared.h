
#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "UObject/ObjectMacros.h"

#include "TextInjectionShared.generated.h"

class UDataDefinition;

// Contextual data that can be used when attempting to resolve text tags
USTRUCT( BlueprintType, meta = (HasNativeMake = "/Script/StarfireUI.TextInjectionUtilities:MakeContext") )
struct STARFIREUI_API FTextTagContexts
{
	GENERATED_BODY( )
public:
	// Constructors
	FTextTagContexts( ) = default;
	FTextTagContexts( const FTextTagContexts & ) = default;
	FTextTagContexts( const TArray< const UObject* > &S, const TArray< const UDataDefinition* > &D ) :
		States( S ), Definitions( D ) { }
	explicit FTextTagContexts( const TArray< const UObject* > &S ) : States( S ) { }
	explicit FTextTagContexts( const TArray< const UDataDefinition* > &D ) : Definitions( D ) { }

	// The contextual information that is represented by state object (runtime) data
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray< const UObject* > States;

	// The contextual information that is represented by definition (static) data
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TArray< const UDataDefinition* > Definitions;

	// Persistence of the scope setting during the expansion of resolvers. not used during resolution
	FString Scope;
};