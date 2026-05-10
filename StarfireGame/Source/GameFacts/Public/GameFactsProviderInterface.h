
#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameFactsProviderInterface.generated.h"

struct FGameplayTagContainer;

// Class data for fact provider interface
UINTERFACE( BlueprintType, MinimalAPI )
class UGameFactsProvider : public UInterface
{
	GENERATED_BODY()
};

// Interface that can be implemented to be considered a type that provides "game fact" tags that can be used to drive gameplay
class GAMEFACTS_API IGameFactsProvider
{
	GENERATED_BODY()
protected:
	// Add facts from the provider to the provided container
	UFUNCTION( BlueprintNativeEvent, Category = "Game Facts" )
	void AppendGameFacts( UPARAM(ref) FGameplayTagContainer &OutFacts ) const;
	virtual void AppendGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const { }

	// Add facts based on the relation between the provider and the input object
	UFUNCTION( BlueprintNativeEvent, Category = "Game Facts" )
	void AppendRelatedGameFacts( const UObject* Other, UPARAM(ref) FGameplayTagContainer &OutFacts ) const;
	virtual void AppendRelatedGameFacts_Implementation( const UObject *Other, FGameplayTagContainer &OutFacts ) const { }

	// After gathering all facts for this object, provide an opportunity to adjust the collection based on the overall collection
	UFUNCTION( BlueprintNativeEvent, Category = "Game Facts" )
	void PostProcessGameFacts( FGameplayTagContainer &OutFacts ) const;
	virtual void PostProcessGameFacts_Implementation( FGameplayTagContainer &OutFacts ) const { }

	friend class UGameFactsProviderUtilities;
};

// The public utilities that can be used to correctly gather the game facts for a single object
UCLASS( )
class GAMEFACTS_API UGameFactsProviderUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Add the facts from an object (maybe in relation to another object) to an existing tag container
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (AdvancedDisplay = "Other", HidePinAssetPicker = "Other") )
	static void AppendGameFacts( const TScriptInterface< IGameFactsProvider > &Target, UPARAM(ref) FGameplayTagContainer &OutFacts, const UObject *Other = nullptr );

	// Add the facts from an object (maybe in relation to another object) to an existing tag container
	static void AppendGameFacts( const UObject *Object, FGameplayTagContainer &OutFacts, const UObject *Other = nullptr );

	// Retrieve the facts for a single object (maybe in relation to another object)
	[[nodiscard]] static FGameplayTagContainer GetGameFacts( const TScriptInterface< IGameFactsProvider > &Target, const UObject *Other = nullptr );

	// Retrieve the facts for a single object (maybe in relation to another object)
	[[nodiscard]] static FGameplayTagContainer GetGameFacts( const UObject *Object, const UObject *Other = nullptr );

private:
	// Add the facts from an object (maybe in relation to another object) to an existing tag container
	UFUNCTION( BlueprintCallable, Category = "Game Facts", DisplayName = "Append Game Fact (Message)", meta = (AdvancedDisplay = "Other", HidePinAssetPicker = "Other") )
	static void AppendGameFacts_Message( const UObject *Object, UPARAM(ref) FGameplayTagContainer &OutFacts, const UObject *Other );

	// Retrieve the facts for a single object (maybe in relation to another object)
	UFUNCTION( BlueprintCallable, Category = "Game Facts", meta = (AdvancedDisplay = "Other", HidePinAssetPicker = "Other") )
	static void GetGameFacts( const TScriptInterface< IGameFactsProvider > &Target, FGameplayTagContainer &Facts, const UObject *Other );

	// Retrieve the facts for a single object (maybe in relation to another object)
	UFUNCTION( BlueprintCallable, Category = "Game Facts", DisplayName = "Get Game Facts (Message)", meta = (AdvancedDisplay = "Other", HidePinAssetPicker = "Other") )
	static void GetGameFacts_Message( const UObject *Target, FGameplayTagContainer &Facts, const UObject *Other );

	// Utility for gathering game facts from all components of an actor that implement the Game Facts Provider interface
	static void AppendActorComponentGameFacts( const AActor *Actor, FGameplayTagContainer &OutFacts, const UObject *Other );
};