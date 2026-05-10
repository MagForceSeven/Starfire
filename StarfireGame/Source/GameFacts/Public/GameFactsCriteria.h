
#pragma once

#include "UObject/ObjectMacros.h"

#include "GameplayTagContainer.h"

#include "GameFactsCriteria.generated.h"

class IGameFactsProvider;

// Utility structure that can be used for configuring conditions for matching against runtime fact collections
USTRUCT( BlueprintType )
struct GAMEFACTS_API FGameFactsCriteria
{
	GENERATED_BODY( )
public:
	// Facts that must all be present to be considered a match
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FGameplayTagContainer RequiresAll;

	// Facts that must have at least one present to be considered a match (if any tags are specified)
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FGameplayTagContainer RequiresAny;
	
	// Facts that must all be not-present to be considered a match
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FGameplayTagContainer RequiresNone;

	// If configured with any queries, the overall query must succeed to be considered a match
	UPROPERTY( EditAnywhere, BlueprintReadWrite, AdvancedDisplay )
	FGameplayTagQuery CustomQuery;

	// Evaluate various inputs that are/have fact collection to determine if they match the criteria conditions
	[[nodiscard]] bool Evaluate( const UObject *Target ) const;
	[[nodiscard]] bool Evaluate( const TScriptInterface< IGameFactsProvider > &Target ) const;
	[[nodiscard]] bool Evaluate( const FGameplayTagContainer &Facts ) const;

	// Check if any configuration is present (if empty, Evaluate will return true for any fact collection)
	[[nodiscard]] bool IsEmpty( ) const;

	// Convert the data into a string for any debug output params
	[[nodiscard]] FString ToString( ) const;
};