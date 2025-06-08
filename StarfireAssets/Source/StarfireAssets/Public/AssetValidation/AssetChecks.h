
#pragma once

class UObject;

class FKismetCompilerContext;

namespace AssetChecks
{
	#define AC_Message( ... ) MessageImpl( __FILE__, __LINE__, __VA_ARGS__ )

	// Simple asset check of a direct message
	STARFIREASSETS_API void MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const UObject *WorldContext, const FKismetCompilerContext *CompilerContext = nullptr );

	// Asset check with additional object information that should be appended after the message
	STARFIREASSETS_API void MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const UObject *SecondaryReference, const UObject *WorldContext, const FKismetCompilerContext *CompilerContext = nullptr );

	// Super-user version. Formatted string with "{<ArrayIndex>}" specifiers.
	//	Appropriate Objects or Object names from SecondaryReferences will be inserted into Message
	STARFIREASSETS_API void MessageImpl( const char *File, int Line, const UObject *Asset, const FString &Message, const TArray< const UObject* > &SecondaryReferences, const UObject *WorldContext, const FKismetCompilerContext *CompilerContext = nullptr );

	// Temporarily change the MessageLog channel that check messages are written to
	struct STARFIREASSETS_API FScopedChecksChannel
	{
		explicit FScopedChecksChannel( const FString &OverrideChannel );
		~FScopedChecksChannel( );
	};
}