
#include "StarfireFader.h"

#include "StarfireUILayout.h"

// Gameplay Tags
#include "GameplayTagContainer.h"

// UMG
#include "Components/Image.h"

// Engine
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireFader)

UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFader::FaderTag_Debug, "FaderID.Debug", "A key for debug fade controls" );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( UStarfireFader::FaderTag_Error, "FaderID.Error", "A key for fallback error handling" );

namespace
{
	FSlateColor FadeColor = FLinearColor::Black;
}

void UStarfireFader::Initialize( FSubsystemCollectionBase &Collection )
{
	Super::Initialize( Collection );

	bInitialized = true;

	UStarfireUILayout::OnLayoutAdded.AddWeakLambda( this,[ this ]( const UStarfireUILayout* Layout ) -> void
	{
		const auto FadeOverlay = Layout->GetFadeOverlay( );
		Overlays.Add( FadeOverlay );

		FadeOverlay->SetBrushTintColor( FadeColor );
	} );

	UStarfireUILayout::OnLayoutRemoved.AddWeakLambda( this,[ this ]( const UStarfireUILayout* Layout ) -> void
	{
		const auto FadeOverlay = Layout->GetFadeOverlay( );
		Overlays.Remove( FadeOverlay );
	} );
}

void UStarfireFader::Tick( float DeltaTime )
{
	const auto FadeSpeed = 1.0f / FadeTime;

	for (auto& [ Overlay, FadeState ] : Overlays)
	{
		const auto DesiredValue = FadeState.IDs.IsEmpty( ) ? 0.0f : 1.0f;
		const auto CurrentValue = Overlay->GetRenderOpacity( );

		if (FMath::IsNearlyEqual( CurrentValue, DesiredValue ))
			continue;
		
		const auto NewValue = FMath::FInterpConstantTo( CurrentValue, DesiredValue, DeltaTime, FadeSpeed );

		Overlay->SetRenderOpacity( NewValue );

		if (!FMath::IsNearlyEqual( NewValue, DesiredValue ))
			continue;

		FadeState.OnFadeComplete.Broadcast( );
		FadeState.OnFadeComplete.Clear( );
	}
}

void UStarfireFader::Deinitialize( )
{
	bInitialized = false;

	UStarfireUILayout::OnLayoutAdded.RemoveAll( this );
	UStarfireUILayout::OnLayoutRemoved.RemoveAll( this );
	
	Super::Deinitialize( );
}

void UStarfireFader::StartFade( const UObject *WorldContext, const FGameplayTag &ID, bool bImmediate, const FFadeCompleteDelegate &OnFadeComplete )
{
	const auto Subsystem = GetSubsystem( WorldContext );
	if (Subsystem == nullptr)
		return;
	
	const auto Layout = UStarfireUILayout::GetPrimaryGameLayout( WorldContext );
	const auto FadeOverlay = Layout->GetFadeOverlay( );

	auto &FadeState = Subsystem->Overlays.FindChecked( FadeOverlay );

	if (OnFadeComplete.IsBound( ))
		FadeState.OnFadeComplete.AddLambda( [ OnFadeComplete ]( ) -> void { OnFadeComplete.Execute( ); } );

	FadeState.IDs.Push( ID );

	if (bImmediate)
	{
		FadeOverlay->SetRenderOpacity( 1.0f );

		FadeState.OnFadeComplete.Broadcast( );
		FadeState.OnFadeComplete.Clear( );
	}
}

void UStarfireFader::EndFade( const UObject *WorldContext, const FGameplayTag &ID, bool bImmediate, const FFadeCompleteDelegate &OnFadeComplete )
{
	const auto Subsystem = GetSubsystem( WorldContext );
	if (Subsystem == nullptr)
		return;
	
	const auto Layout = UStarfireUILayout::GetPrimaryGameLayout( WorldContext );
	const auto FadeOverlay = Layout->GetFadeOverlay( );

	auto &FadeState = Subsystem->Overlays.FindChecked( FadeOverlay );

	if (OnFadeComplete.IsBound( ))
		FadeState.OnFadeComplete.AddLambda( [ OnFadeComplete ]( ) -> void { OnFadeComplete.Execute( ); } );

	FadeState.IDs.Remove( ID );

	if (FadeState.IDs.IsEmpty( ) && bImmediate)
	{
		FadeOverlay->SetRenderOpacity( 0.0f );

		FadeState.OnFadeComplete.Broadcast( );
		FadeState.OnFadeComplete.Clear( );
	}
}

void UStarfireFader::StartFade_BP( const UObject *WorldContext, FGameplayTag ID, bool bImmediate )
{
	if (!ensureAlways( ID.IsValid(  ) ))
		ID = FaderTag_Error;
	
	StartFade( WorldContext, ID, bImmediate );
}

void UStarfireFader::EndFade_BP( const UObject *WorldContext, FGameplayTag ID, bool bImmediate )
{
	if (!ensureAlways( ID.IsValid(  ) ))
		ID = FaderTag_Error;

	EndFade( WorldContext, ID, bImmediate );
}

ETickableTickType UStarfireFader::GetTickableTickType( ) const 
{
	// If this is a template or has not been initialized yet, set to never tick, and it will be enabled when it is initialized
	if (IsTemplate() || !bInitialized)
	{
		return ETickableTickType::Never;
	}

	// Otherwise Tick
	return ETickableTickType::Always;
}

// Starfire Utilities
#include "Misc/ExecSF.h"

using namespace ExecSF_Params;
struct FFaderExecs : public FExecSF
{
	FFaderExecs( )
	{
		AddExec( TEXT( "Starfire.Fades.FadeOut" ), TEXT( "Apply a fade on top of the game (optional bool for immediate fade)" ), FExecDelegate::CreateStatic( &FFaderExecs::FadeOut ) );
		AddExec( TEXT( "Starfire.Fades.FadeIn" ), TEXT( "Force any in progress fade to be removed (optional bool for immediate fade)" ), FExecDelegate::CreateStatic( &FFaderExecs::FadeIn ) );
		AddExec( TEXT( "Starfire.Fades.EnableFadeDebug" ), TEXT( "Apply a debug color to fades & display the active IDs" ), FExecDelegate::CreateStatic( &FFaderExecs::DebugFade ) );
	}

	static void FadeOut( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool bImmediateFade = false;
		GetParams( Cmd, bImmediateFade );

		UStarfireFader::StartFade( World, UStarfireFader::FaderTag_Debug, bImmediateFade );
	}

	static void FadeIn( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool bImmediateFade = false;
		GetParams( Cmd, bImmediateFade );

		const auto Subsystem = UStarfireFader::GetSubsystem( World );
		if (Subsystem == nullptr)
			return;

		const auto Layout = UStarfireUILayout::GetPrimaryGameLayout( World );
		const auto FadeOverlay = Layout->GetFadeOverlay( );

		auto &FadeState = Subsystem->Overlays.FindChecked( FadeOverlay );

		if (FadeState.IDs.IsEmpty( ))
			return;

		FadeState.IDs.Remove( UStarfireFader::FaderTag_Debug );

		if (!FadeState.IDs.IsEmpty( ))
		{
			Ar.Log( TEXT( "Ending Fade with active fade ids:\n" ) );

			for (const auto &ID : FadeState.IDs)
				Ar.Logf( TEXT( "%s\n" ), *ID.ToString( ) );

			FadeState.IDs.Empty( );
		}

		if (bImmediateFade)
			FadeOverlay->SetRenderOpacity( 0.0f );
	}

	static void DebugFade( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		bool bEnableDebugFade = false;
		if (GetParams( Cmd, bEnableDebugFade ) == 0)
			return;

		FadeColor = bEnableDebugFade ? FColor::Magenta : FLinearColor::Black;

		const auto Subsystem = UStarfireFader::GetSubsystem( World );
		if (Subsystem == nullptr)
			return;

		for (const auto& [ Overlay, IDs ] : Subsystem->Overlays)
			Overlay->SetBrushTintColor( FadeColor );
	}

} GFaderExecs;