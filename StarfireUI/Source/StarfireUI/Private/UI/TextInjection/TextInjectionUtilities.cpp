
#include "UI/TextInjection/TextInjectionUtilities.h"

#include "UI/TextInjection/TextTagResolver.h"
#include "UI/TextInjection/TextInjectionShared.h"

#include "DataDefinitions/DataDefinitionSource.h"

#include "Kismet/BlueprintUtilitiesSF.h"

// Core
#include "Internationalization/Regex.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TextInjectionUtilities)

TArray< FString > UTextInjectionUtilities::ParseTags( const FText &Text )
{
	if (Text.IsEmpty( ))
		return { };

	static const FRegexPattern TagsPattern( "\\{(.+?)\\}" );
	FRegexMatcher Matcher( TagsPattern, Text.ToString( ) );

	TArray< FString > Results;
	while (Matcher.FindNext( ))
		Results.Emplace( Matcher.GetCaptureGroup( 1 ) );

	return Results;
}

// A custom key resolvers for leveraging a set during resolution
// But the struct shouldn't normally have the necessary operators overloaded
struct FResolverKeyFuncs : BaseKeyFuncs< FTagResolver, FTagResolver >
{
	typedef TTypeTraits<FTagResolver>::ConstPointerType KeyInitType;
	typedef TCallTraits<FTagResolver>::ParamType ElementInitType;

	/**
	 * @return The key used to index the given element.
	 */
	[[nodiscard]] static UE_FORCEINLINE_HINT KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element;
	}

	/**
	 * @return True if the keys match.
	 */
	[[nodiscard]] static UE_FORCEINLINE_HINT bool Matches(KeyInitType A, KeyInitType B)
	{
		if (A.Resolver != B.Resolver)
			return false;

		if (A.Scope != B.Scope)
			return false;
		
		return true;
	}

	/** Calculates a hash index for a key. */
	[[nodiscard]] static UE_FORCEINLINE_HINT uint32 GetKeyHash(KeyInitType Key)
	{
		return GetTypeHash( Key.Resolver );
	}
};

FText UTextInjectionUtilities::ResolveTextTags( const FText &Text, const TScriptInterface< const ITextTagResolver >& Resolver, FTextTagContexts Contexts, const UObject *WorldContext )
{
	if (!ensureAlways( Resolver != nullptr ))
		return Text;

	// 1) Convert the input text into a mapping of keys to strings
	const auto Tags = ParseTags( Text );
	if (Tags.IsEmpty( ))
		return Text;

	TMap< FString, FText > TagMap;
	for (const auto &T : Tags)
		TagMap.Add( T, FText( ) );

	// 2) Preprocess the Context Data
	Contexts.States.Remove( nullptr );
	Contexts.Definitions.Remove( nullptr );

	for (auto S : Contexts.States)
	{
		if (S->Implements< UDataDefinitionSource >( ))
		{
			if (auto D = IDataDefinitionSource::Execute_GetSourceDefinition( S ))
				Contexts.Definitions.AddUnique( D );
		}
	}

	Resolver->PreResolveUpdateContexts( Contexts, WorldContext );

	// 3) Build a list of all the objects that may have information that will resolve possible tags into values
	TArray< FTagResolver > Resolvers;
	TSet< FTagResolver, FResolverKeyFuncs > SeenResolvers;

	Resolvers.Emplace( Resolver.GetObject( ), Contexts );
	SeenResolvers.Add( FTagResolver( Resolver.GetObject( ), { } ) );

	TArray< FTagResolver > SubResolvers;
	for (int idx = 0; idx < Resolvers.Num( ); ++idx)
	{
		const auto &R = Resolvers[ idx ];
		CastChecked< ITextTagResolver >( R.Resolver )->AppendAdditionalResolvers( SubResolvers, R.Contexts, WorldContext );

		for (const auto &Sub : SubResolvers )
		{
			if (Sub.Resolver == nullptr)
				continue;
			if (!Sub.Resolver->Implements< UTextTagResolver >( ))
				continue;
			if (SeenResolvers.Contains( Sub ))
				continue;

			SeenResolvers.Add( Sub );
			Resolvers.Push( Sub );
		}

		SubResolvers.Reset( );
	}

	// 4) A simple dispatch of ITextTagResolver::ResolveTextTags is made to all the resolvers found in Step 3.
	for (const auto &R : Resolvers)
		CastChecked< ITextTagResolver >( R.Resolver )->ResolveTextTags( FTextTags( R.Scope, TagMap ), R.Contexts, WorldContext );

	// 5) A post-process step is performed on the tag-value map to convert from the simple TMap< FString, FString > to an FFormatNamedArguments instance
	FFormatNamedArguments FormatArgs;
	for (const auto &Entry : TagMap)
	{
		if (Entry.Value.IsEmpty( ))
		{
			auto FailedText = FText::Format( FText::FromString( TEXT( "[{0}]" ) ), FText::FromString( Entry.Key ) );
			FormatArgs.Add( Entry.Key, FailedText );
		}
		else
		{
			FormatArgs.Add( Entry.Key, Entry.Value );
		}
	}

	// 6) Finally FText::Format is called using the original input Text and the FFormatNamedArguments from Step 5.
	const auto FinalText = FText::Format( Text, FormatArgs );

	return FinalText;
}

FTextTagContexts UTextInjectionUtilities::MakeContext( const TArray< UObject* > &States, const TArray< UDataDefinition* > &Definitions )
{
	return FTextTagContexts( NativeCompatibilityCast( States ), NativeCompatibilityCast( Definitions ) );
}

//*********************************************************************
// Custom Thunk Implementations
//*********************************************************************

DEFINE_FUNCTION( UTextInjectionUtilities::execResolveTextTags )
{
	// ReSharper disable CppLocalVariableMayBeConst
	P_GET_PROPERTY_REF( FTextProperty, Z_Param_Out_Text );
	P_GET_TINTERFACE_REF( ITextTagResolver, Z_Param_Resolver );
	P_GET_STRUCT( FTextTagContexts, Z_Param_Contexts );
	P_GET_OBJECT( UObject, Z_Param_WorldContext );
	P_FINISH;
	// ReSharper restore CppLocalVariableMayBeConst

	P_NATIVE_BEGIN;

	*(FText*)Z_Param__Result = UTextInjectionUtilities::ResolveTextTags( Z_Param_Out_Text, Z_Param_Resolver, Z_Param_Contexts, Z_Param_WorldContext );

	P_NATIVE_END;
}

DEFINE_FUNCTION( UTextInjectionUtilities::execMakeContext )
{
	// ReSharper disable CppLocalVariableMayBeConst
	P_GET_TARRAY_REF( UObject*, Z_Param_Out_States );
	P_GET_TARRAY_REF( UDataDefinition*, Z_Param_Out_Definitions );
	P_FINISH;
	// ReSharper restore CppLocalVariableMayBeConst

	P_NATIVE_BEGIN;

	*(FTextTagContexts*)Z_Param__Result = UTextInjectionUtilities::MakeContext( Z_Param_Out_States, Z_Param_Out_Definitions );

	P_NATIVE_END;
}