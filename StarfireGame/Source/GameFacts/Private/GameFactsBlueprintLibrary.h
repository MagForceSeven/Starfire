
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameFactsBlueprintLibrary.generated.h"

struct FGameFactsCriteria;
struct FGameplayTagContainer;
class IGameFactsProvider;

// Collection of blueprint utilities for interacting with game facts
UCLASS( )
class UGameFactsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:

private:
	// Evaluate an object against a fact criteria configuration
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (DisplayName = "Evaluate Facts Criteria against Object", HidePinAssetPicker = "Target") )
	static bool EvaluateCriteria_Object( const FGameFactsCriteria &Criteria, UObject *Target );

	// Evaluate a against a fact criteria configuration
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (DisplayName = "Evaluate Facts Criteria against Interface") )
	static bool EvaluateCriteria_Target( const FGameFactsCriteria &Criteria, const TScriptInterface< IGameFactsProvider > &Target );

	// Evaluate a collection of tags against a fact criteria configuration
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (DisplayName = "Evaluate Facts Criteria against Facts") )
	static bool EvaluateCriteria_Tags( const FGameFactsCriteria &Criteria, const FGameplayTagContainer &Facts );
};