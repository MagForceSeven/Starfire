
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UI/TextInjection/TextInjectionShared.h"

#include "TextInjectionUtilities.generated.h"

class ITextTagResolver;

class UDataDefinition;

struct FTextTagContexts;

// Utilities for managing the text injection process
UCLASS( )
class STARFIREUI_API UTextInjectionUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:

	// Attempts to replace "{ }" sections of Text using the provided context data
	UFUNCTION( BlueprintCallable, CustomThunk, Category = "UI|TextInjection", meta = (WorldContext = "WorldContext", AutoCreateRefTerm = "Text") )
	[[nodiscard]] static FText ResolveTextTags( const FText &Text, const TScriptInterface< const ITextTagResolver >& Resolver, FTextTagContexts Contexts, const UObject *WorldContext );

	// Determine the strings that are surrounded by { }
	[[nodiscard]] static TArray< FString > ParseTags( const FText &Text );

protected:

	// Custom function to construct the context data required for blueprint to properly construct a text resolution context
	UFUNCTION( BlueprintCallable, BlueprintPure = true, Category = "UI|TextInjection", meta = (NativeMakeFunc, AutoCreateRefTerm = "States, Definitions"), CustomThunk )
	static FTextTagContexts MakeContext( const TArray< UObject* > &States, const TArray< UDataDefinition* > &Definitions );

	DECLARE_FUNCTION( execResolveTextTags );
	DECLARE_FUNCTION( execMakeContext );
};

/*
--------  An algorithm for inserting gameplay values (numbers, strings, etc.) into display strings ----------------------

For example purposes, the inputs to ResolveTextTags will be:
	Text = "{Source:Name} deals {Source:Stats:Attack} damage to {Target:Name}"
	Resolver = A Data Asset of some type that is the static configuration of an ability (most likely the source of the Text parameter)
	Contexts = State array includes a runtime UObject/AActor representing the runtime state of an ability
			   State array includes a runtime UObject/AActor representing a character that is the target
			   No Definition entries
	(Some of this is a little contrived for the purposes of illustration and not meant to be prescriptive about how data is organized or related)

	(If Data Definitions from the Starfire Assets plugin aren't being used, this can be changed to Data Asset instead.
	The intent is there tends to be a meaningful difference for resolvers in looking up data from runtime objects vs static asset data.)

	1) Convert the input text into a mapping of keys to empty strings.
		Example: The following map keys would be extracted: "Source:Name", "Source:Stats:Attack" & "Target:Name"
	2) Preprocess the Context Data in a few different ways:
		a) Remove nullptr/!IsValid ptrs just in case - allow the resolvers to assume valid pointers.
		b) Leverage an interface to add associated data assets for each of the Context's State data to the Definitions data.
			(The goal of the interface is to identify a "core" data asset for a runtime object, such as an ability state returning the static data asset of its behavior)
			(It is not a requirement that the state objects provided as contexts implement the interface, but it's convenient to have both in the Context without the
				caller of ResolveTextTags having to add to both arrays themselves)
			(If not using Starfire Assets, this step/interface could be removed)
		c) Give the Resolver a chance (through ITextTagResolver::PreResolveUpdateContexts) to update the context data in any ways that make sense
	3) Build a list of all the objects that may have information that will resolve possible tags into values
		a) This list starts with the input Resolver
		b) For every list entry, ITextTagResolver::AppendAdditionalResolvers is called to get more resolvers
			i) The resolver appends (an) object(s) that implements the interface
			ii) It also associates an optional "Scope" string
			iii) This scope string is appended to whatever scope may have been used to add this resolver to the list
			iv) The "Scope" is used to allow the same types of objects to replace different elements that may need similar information
					In our example this is would be so that the same 'Character' type but different instances would be able to fill in
					"Source:Name" vs "Target:Name".
		c) Every new resolver (object + scope pair) is added to the list until no new resolvers have been added
			i) This means the same object may participate in the resolution process multiple times, with different scopes
			ii) While this sounds like unnecessary repetition, it's important to allow contributions from multiple reference paths
			iii) Still, it does remove some repetition and why Step 3 was separated out from Step 4 at all
					In the original implementations of ITextTagResolver::ResolveTextTags would just call the same function on other
						other resolvers resulting in lots of duplicate resolver processing. Now ::ResolveTextTags only
						concerns itself with the local data replacements
			Example: Starting with the Ability Asset, it would add the Ability State from the context as a resolver
					The Ability State would then add the character that owned the ability with the "Source" scope
						and it would add the target character from the context with the "Target" scope
					Since the source and target are both Characters, they'll both add their own Stat tracking component as
						a resolver, with the scope "Stats". Since they were both added with scope strings, the components
						would be added to the list with "Source:Stats" and "Target:Stats" scopes respectively.
					Further resolvers may get added by any of these participants, but for the example this is sufficient.
		4) A simple dispatch of ITextTagResolver::ResolveTextTags is made to all the resolvers found in Step 3.
			i) Implementors use either FTextTags::FindTag or FTextTags::AssignToTag to update the Tags reference parameter
			ii) The first resolver to provide a value for a key wins. Any further resolvers will treat the tag as a key that is not present
				This is where the importance of the scoping comes in. Otherwise it's difficult to include data across multiple objects
			iii) The scoping is handled by the FTextTags structure. The resolver can use just the Tag portion as if there were no scopes involved
			Example: The Character object would look to replace the "Name" tag. Since each Character object was added with a unique scope
				each Character will replace only the "Name" specific to the Scope they were added with.
				And the Stat tracking components would look for Tags matching each of the Stats they track (or some mapping from them to tags)
					like "Attack" or "HP". So in this case only the component added with the scope "Source:Stats" would find something
					replaceable for any of its values
		5) A post-process step is performed on the tag-value map to convert from the simple TMap< FString, FString > to an FFormatNamedArguments instance
			i) Any tag that still has an empty string value has a value generated that is the tag in square brackets so
				"Source:Stats:Attack" would show up in the final output string as "[Source:Stats:Attack]".
				This makes it clear in the UI when this injection process has failed vs when wrong value being inserted
		6) Finally FText::Format is called using the original input Text and the FFormatNamedArguments from Step 5.
			i) FText::Format is case sensitive while FString keys to TMap are not (for some reason) so repeated insertions
				of the same tag should be handled with care.
				Example: This means "Source:Name" and "Source:name" will result in one of the two tags not being replaced
			ii) The case insensitivity of TMap does work to the advantage for non-repeated tags though as it means
				resolvers don't have to worry about replacing variations of case in the tags.
 */