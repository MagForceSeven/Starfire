
#include "AssetValidation/AssetChecks.h"

#include "Debug/Redscreen.h"

#if WITH_EDITOR
	// Blueprint Graph
	#include "K2Node.h"

	// Unreal Ed
	#include "EdGraphToken.h"

	// Kismet Compiler
	#include "KismetCompiler.h"

	// Core UObject
	#include "Misc/UObjectToken.h"

	// Core
	#include "Logging/MessageLog.h"
#endif

// Core
#include "Internationalization/Regex.h"

static FString OverrideCheckChannel;

AssetChecks::FScopedChecksChannel::FScopedChecksChannel( const FString &OverrideChannel )
{
	check( OverrideCheckChannel.IsEmpty( ) );
	OverrideCheckChannel = OverrideChannel;
}

AssetChecks::FScopedChecksChannel::~FScopedChecksChannel( )
{
	OverrideCheckChannel.Empty( );
}

#if WITH_EDITOR
TSharedRef< FTokenizedMessage > MessageBase( const UObject *Asset, FKismetCompilerContext *CompilerContext )
{
	if (!Asset->IsA< UK2Node >( ))
	{
		FMessageLog AssetCheckChannel( OverrideCheckChannel.IsEmpty( ) ? TEXT( "AssetCheck" ) : *OverrideCheckChannel );
		return AssetCheckChannel.Warning( )->AddToken( FUObjectToken::Create( Asset ) );
	}

	check( CompilerContext != nullptr );

	auto Line = FTokenizedMessage::Create( EMessageSeverity::Error );

	TArray< UEdGraphNode* > Ignored;
	FEdGraphToken::Create( Asset, &CompilerContext->MessageLog, *Line, Ignored );

	CompilerContext->MessageLog.Messages.Push( Line );

	return Line;
}
#endif

void AssetChecks::MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	check( Asset != nullptr );
	check( !Message.IsEmpty( ) );

#if WITH_EDITOR
	MessageBase( Asset, CompilerContext )
		->AddToken( FTextToken::Create( FText::FromString( ": " + Message ) ) );
#endif
	RedscreenImpl( WorldContext, false, File, Line, TEXT( "%s: %s" ), *Asset->GetName( ), *Message );
}

void AssetChecks::MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const UObject *SecondaryReference, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	check( Asset != nullptr );
	check( !Message.IsEmpty( ) );
	check( SecondaryReference != nullptr );

#if WITH_EDITOR
	MessageBase( Asset, CompilerContext )
		->AddToken( FTextToken::Create( FText::FromString( ": " + Message + " - " ) ) )
		->AddToken( FUObjectToken::Create( SecondaryReference ) );
#endif
	RedscreenImpl( WorldContext, false, File, Line, TEXT( "%s: %s - %s" ), *Asset->GetName( ), *Message, *SecondaryReference->GetName( ) );
}

void AssetChecks::MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const TArray< const UObject* > &SecondaryReferences, const UObject *WorldContext, FKismetCompilerContext *CompilerContext )
{
	check( Asset != nullptr );
	check( !Message.IsEmpty( ) );
	check( SecondaryReferences.Find( nullptr ) == INDEX_NONE );

	FFormatNamedArguments FormatArgs;
#if WITH_EDITOR
	int PrevEnd = 0;

	const auto Warning = MessageBase( Asset, CompilerContext );
#endif

	static const FRegexPattern TagsPattern( "\\{(.+?)\\}" );
	FRegexMatcher Matcher( TagsPattern, Message );
	while (Matcher.FindNext( ))
	{
		const auto Match = Matcher.GetCaptureGroup( 1 );
		ensureAlways( Match.IsNumeric( ) );

		// Try to match up the format specifier to an entry in the array
		const auto Index = FCString::Atoi( *Match );
		if (Match.IsNumeric( ) && SecondaryReferences.IsValidIndex( Index ))
			FormatArgs.Add( Match, FText::FromString( SecondaryReferences[ Index ]->GetName( ) ) );
		else
			FormatArgs.Add( Match, FText::FromString( TEXT( "INVALID" ) ) );

#if WITH_EDITOR
		// Create a string from the end of the previous match to just before the beginning of the current match
		const auto PriorToMatchBegin = Matcher.GetMatchBeginning( ) - 1;
		if (PrevEnd != PriorToMatchBegin)
			Warning->AddToken( FTextToken::Create( FText::FromString( Message.Mid( PrevEnd, PriorToMatchBegin - PrevEnd ) ) ) );
		PrevEnd = Matcher.GetMatchEnding( ) + 1; // move just past the end if this match

		// Try to insert an element from the array into the warning
		if (Match.IsNumeric( ) && SecondaryReferences.IsValidIndex( Index ))
			Warning->AddToken( FUObjectToken::Create( SecondaryReferences[ Index ] ) );
		else
			Warning->AddToken( FTextToken::Create( FText::FromString( TEXT( "INVALID" ) ) ) );
#endif
	}

	// Do the text replacement and dispatch as a redscreen
	const auto FinalText = FText::Format( FText::FromString( Message ), FormatArgs ).ToString( );
	RedscreenImpl( WorldContext, false, File, Line, TEXT( "%s: %s" ), *Asset->GetName( ), *FinalText );

#if WITH_EDITOR
	// Create a string from the end of the previous match to just before the beginning of the current match
	if (PrevEnd != Message.Len( ))
		Warning->AddToken( FTextToken::Create( FText::FromString( Message.Mid( PrevEnd, Message.Len( ) - PrevEnd ) ) ) );
#endif
}