
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "NativeGameplayTags_SF.h"

#include "StarfireFader.generated.h"

struct FGameplayTag;
class UImage;

DECLARE_STATS_GROUP( TEXT( "StarfireFader" ), STATGROUP_StarfireFader, STATCAT_Advanced );

// Custom fader solution - camera fades don't affect widgets
UCLASS( Config = "Game" )
class STARFIREUI_API UStarfireFader : public UGameInstanceSubsystem, public FTickableGameObject, public TSubsystemNativeAccessors< UStarfireFader >
{
	GENERATED_BODY( )
public:
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(FaderTag_Debug);
	UE_DECLARE_GAMEPLAY_TAG_SCOPED(FaderTag_Error);
	
	// Tickable Game Object API
	ETickableTickType GetTickableTickType( ) const override;
	void Tick( float DeltaTime ) override;
	bool IsTickableWhenPaused() const override { return false; }
	UWorld* GetTickableGameObjectWorld( void ) const override { return Super::GetWorld( ); }
	TStatId GetStatId( ) const override { RETURN_QUICK_DECLARE_CYCLE_STAT( UStarfireFader, STATGROUP_StarfireFader ); }

	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;

	DECLARE_DELEGATE( FFadeCompleteDelegate );

	// Apply a fade to the screen over time
	static void StartFade( const UObject *WorldContext, const FGameplayTag &ID, bool bImmediate = false, const FFadeCompleteDelegate& OnFadeComplete = { } );

	// Remove a fade from the screen over time
	static void EndFade( const UObject *WorldContext, const FGameplayTag &ID, bool bImmediate = false, const FFadeCompleteDelegate& OnFadeComplete = { } );
	
private:
	friend struct FFaderExecs;
	
	// Amount of time to take when fading
	UPROPERTY( Config )
	float FadeTime = 1.5f;

	DECLARE_MULTICAST_DELEGATE( FFadeCompleteMultiCastDelegate );

	// The data for each widget being used to fade
	struct FFadeState
	{
		// The tags that have request that the widget be faded out
		TArray< FGameplayTag > IDs;

		// Callbacks to execute when the overlay completes the fade
		FFadeCompleteMultiCastDelegate OnFadeComplete;
	};

	// The collection of widgets that can be used to fade the screen
	TMap< TObjectPtr< UImage >, FFadeState > Overlays;

	// Track initialization state to control ticking
	bool bInitialized = false;

private:
	// Apply a fade to the screen over time
	UFUNCTION( BlueprintCallable, Category = "Starfire Fader", DisplayName = "Start Fade", meta = (WorldContext = "WorldContext") )
	static void StartFade_BP( const UObject *WorldContext, UPARAM(meta=(Categories = "FaderID")) FGameplayTag ID, bool bImmediate = false );

	// Remove a fade from the screen over time
	UFUNCTION( BlueprintCallable, Category = "Starfire Fader", DisplayName = "End Fade", meta = (WorldContext = "WorldContext") )
	static void EndFade_BP( const UObject *WorldContext, UPARAM(meta=(Categories = "FaderID")) FGameplayTag ID, bool bImmediate = false );
};