
#include "UI/TextInjection/TextTagResolver.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TextTagResolver)

FTextTags::FTextTags( TMap< FString, FText > &TagMap ) :
	Inner( TagMap )
{
}

FTextTags::FTextTags( const FString &InScope, const FTextTags &Tags ) :
	Scope( InScope ),
	Inner( Tags.Inner )
{
}

FText* FTextTags::FindTag( const FString &Tag ) const
{
	if (!ensureAlways( !Tag.IsEmpty( ) ))
		return nullptr;

	const auto SearchTag = Scope + ":" + Tag;

	const auto Search = Inner.Find( SearchTag );
	if (Search == nullptr)
		return nullptr;

	if (!Search->IsEmpty( ))
		return nullptr;

	return Search;
}

void FTextTags::AssignToTag( const FString &Tag, const FText &Text ) const
{
	if (const auto Search = FindTag( Tag ))
		*Search = Text;
}

FTagResolver::FTagResolver( const UObject *R, const FTextTagContexts &C, const FString &S ) :
	Resolver( R ), Contexts( C )
{
	if (!S.IsEmpty( ))
	{
		if (!Contexts.Scope.IsEmpty( ))
			Contexts.Scope += ":";
		Contexts.Scope += S;

		Scope = Contexts.Scope;
	}
}

void ITextTagResolver::AppendResolvers( TArray< FTagResolver > &outResolvers, const TArray< const UObject* > &Resolvers, const FTextTagContexts &Contexts, const FString &Scope )
{
	outResolvers.Reserve( outResolvers.Num( ) + Resolvers.Num( ) );

	for (auto R : Resolvers)
	{
		if (R != nullptr)
			outResolvers.Emplace( R, Contexts, Scope );
	}
}

void ITextTagResolver::ResolveTextTags( const FTextTags &Tags, const FTextTagContexts &Contexts, const UObject *WorldContext ) const
{
}