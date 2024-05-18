
#include "Debug/Redscreen.h"

#include "Debug/RedscreenManager.h"

// This starting size catches 99.97% of printf calls
static const int STARTING_BUFFER_SIZE = 512;

// Implementation mostly duplicated from FString::PrintfImpl
void RedscreenImpl( const UObject *WorldContext, bool bOnce, const char *File, int Line, const TCHAR *Fmt, ... )
{
	if (WorldContext == nullptr)
		return;

	UWorld *World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return;

	// Ignore redscreens generated in Editor Levels (active or not) or in Editor Previews
	switch (World->WorldType)
	{
		case EWorldType::Editor:
		case EWorldType::EditorPreview:
		case EWorldType::Inactive:
			return;

		default:
			break;
	}

	int32 BufferSize = STARTING_BUFFER_SIZE;
	TCHAR StartingBuffer[ STARTING_BUFFER_SIZE ];
	TCHAR *Buffer = StartingBuffer;
	TCHAR *AllocatedBuffer = nullptr;
	int32 Result = -1;

	while (Result == -1)
	{
		GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize - 1, Fmt, Fmt, Result );

		if (Result == -1)
		{
			BufferSize *= 2;
			AllocatedBuffer = (TCHAR*)FMemory::Realloc( AllocatedBuffer, BufferSize * sizeof( TCHAR ) );
			Buffer = AllocatedBuffer;
		}
	}

	// terminate and convert to something more convenient
	Buffer[ Result ] = 0;
	FString Message( Buffer );

	// add the filename and line to the display string
	Message = FString::Printf( TEXT( "(%s, %d): " ), *FPaths::GetCleanFilename( File ), Line ) + Message;

	const auto RedscreenManager = URedscreenManager::GetSubsystem( WorldContext );
	check( RedscreenManager != nullptr );

	RedscreenManager->AddRedscreen( Message, bOnce );

	if (AllocatedBuffer != nullptr)
		FMemory::Free( AllocatedBuffer );
}

FString GetBlueprintCallerFunctionName( void )
{
#if DO_BLUEPRINT_GUARD
	if (const auto Tracker = FBlueprintContextTracker::TryGet( ))
	{
		const auto RawStack = Tracker->GetCurrentScriptStack( );

		if (RawStack.IsEmpty(  ))
			return TEXT( "Not a Blueprint Call" );

		return RawStack.Last( )->Node->GetName( );
	}
#endif

	return TEXT( "Unknown Blueprint Source" );
}
