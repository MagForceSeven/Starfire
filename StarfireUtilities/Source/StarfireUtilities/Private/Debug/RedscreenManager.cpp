
#include "Debug/RedscreenManager.h"

#include "Debug/RedscreenScreen.h"
#include "Debug/Redscreen.h"

// Projects
#include "Interfaces/IPluginManager.h"

// Core
#include "Logging/StructuredLog.h"

// Engine
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC( LogRedscreen, Warning, Warning );

void Redscreens::Init( const UObject *WorldContext )
{
	const auto Manager = URedscreenManager::GetSubsystem( WorldContext );
	Manager->OnMatchStarted( WorldContext );
}

URedscreenManager* URedscreenManager::GetSubsystem( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return nullptr;

	return World->GetSubsystem< URedscreenManager >( );
}

void URedscreenManager::OnMatchStarted( const UObject *WorldContext )
{
	if (RedscreenWidgetType.IsNull( ))
		return; // redscreens aren't configured
	
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World != nullptr ))
		return;
	
	if (const auto ScreenClass = RedscreenWidgetType.LoadSynchronous( ))
	{
		ScreenWidget = CreateWidget< URedscreenScreen >( World, ScreenClass );
		ScreenWidget->SetVisibility( ESlateVisibility::Hidden );
		
		ScreenWidget->OnClose.AddDynamic( this, &URedscreenManager::OnScreenClosed );

		if (!QueuedMessages.IsEmpty( ))
		{
			bVisible = true;
			ScreenWidget->AddToViewport( INT_MAX );
			ScreenWidget->SetVisibility( ESlateVisibility::HitTestInvisible );

			for (const auto &M : QueuedMessages)
				ScreenWidget->AddMessage( M );
			QueuedMessages.Empty( );
		}
	}
	else
	{
		UE_LOGFMT( LogRedscreen, Warning, "Unable to load redscreen widget class at {0}/{1}", RedscreenWidgetType.GetLongPackageName( ), RedscreenWidgetType.GetAssetName( ) );
	}
}

void URedscreenManager::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	if (RedscreenWidgetType.IsNull( ))
	{
		const auto PluginFolder = FPaths::ConvertRelativePathToFull( IPluginManager::Get( ).FindPlugin( TEXT("StarfireUtilities") )->GetBaseDir( ) );
		const FString SettingsDefault = PluginFolder / TEXT( "Config/Redscreens.ini" );
		if (FPaths::FileExists( SettingsDefault ))
		{
			GConfig->LoadFile( SettingsDefault );
			const FString Path = GConfig->GetStr( TEXT("/Script/StarfireUtilities.RedscreenManager"), TEXT("RedscreenWidgetType"), SettingsDefault );
			GConfig->UnloadFile( SettingsDefault );

			if (!Path.IsEmpty( ))
				RedscreenWidgetType = FSoftObjectPath( Path );
		}
	}
}

void URedscreenManager::Deinitialize( )
{
	ScreenWidget = nullptr;

	Super::Deinitialize( );
}

void URedscreenManager::OnScreenClosed( URedscreenScreen *Screen )
{
	ScreenWidget->SetVisibility( ESlateVisibility::Hidden );
	bVisible = false;
}

void URedscreenManager::AddRedscreen( const FString &Message, bool bOnce )
{
	if (bOnce)
	{
		const auto CRC = FCrc::StrCrc32( *Message );
		if (SeenOnceCRCs.Contains( CRC ))
			return;

		SeenOnceCRCs.Add( CRC );
	}
	
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
	if (ScreenWidget != nullptr)
	{
		if (!bVisible)
		{
			bVisible = true;
			ScreenWidget->AddToViewport( INT_MAX / 2 );
			ScreenWidget->SetVisibility( ESlateVisibility::HitTestInvisible );
		}

		ScreenWidget->AddMessage( Message );
	}
	else
	{
		QueuedMessages.Push( Message );
	}
#endif

	UE_LOGFMT( LogRedscreen, Warning, "{0}", Message );
}

bool URedscreenManager::DoesSupportWorldType( const EWorldType::Type WorldType ) const
{
	return (WorldType == EWorldType::Game) || (WorldType == EWorldType::PIE);
}

#include "Misc/ExecSF.h"

struct FRedscreenTester : public FExecSF
{
	FRedscreenTester( )
	{
		AddExec( TEXT( "Starfire.Redscreens.TestMessage" ), TEXT( "" ), FExecDelegate::CreateStatic( &FRedscreenTester::TestMessage ) );
		AddExec( TEXT( "Starfire.Redscreens.TestRedscreen" ), TEXT( "" ), FExecDelegate::CreateStatic( &FRedscreenTester::TestRedscreen ) );
		AddExec( TEXT( "Starfire.Redscreens.TestRedscreenOnce" ), TEXT( "" ), FExecDelegate::CreateStatic( &FRedscreenTester::TestRedscreenOnce ) );
	}

	static void TestMessage( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		URedscreenManager::GetSubsystem( World )->AddRedscreen( "Test Message", false );
	}

	static void TestRedscreen( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		Redscreen( World, TEXT( "%s" ), TEXT( "Redscreen Test Message" ) );
	}

	static void TestRedscreenOnce( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		RedscreenOnce( World, TEXT( "%s" ), TEXT( "Redscreen Test Message Once" ) );
	}

/*
// If uncommented, these should generate errors
	static void CompileTest( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		Redscreen( Cmd, TEXT( "%s" ), Cmd ); // static assert World Context type incorrect
		Redscreen( World, FString( Cmd ) ); // static assert format string type incorrect
	}
//*/

} GRedscreenTester;