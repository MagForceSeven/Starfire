
#pragma once

#include "Kismet/BlueprintAsyncActionSF.h"

#include "GameplayTagContainer.h"

#include "StarfireFader_Async.generated.h"

// Async handlers for Starfire Fader fade requests
UCLASS( )
class UStarfireFader_Async : public UBlueprintAsyncAction_SF
{
	GENERATED_BODY( )
public:
	// Apply a fade to the screen over time with a callback on completion
	UFUNCTION( BlueprintCallable, DisplayName = "Fade Out (Async)", Category = "Starfire Fader", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UStarfireFader_Async* FadeOut_Async( UObject *WorldContext, UPARAM(meta=(Categories = "FaderID")) FGameplayTag ID );
	
	// Remove a fade from the screen over time with a callback on completion
	UFUNCTION( BlueprintCallable, DisplayName = "Fade In (Async)", Category = "Starfire Fader", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = true) )
	static UStarfireFader_Async* FadeIn_Async( UObject *WorldContext, UPARAM(meta=(Categories = "FaderID")) FGameplayTag ID );

	// Latent callback on fade completion
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FCompletionDelegate );
	UPROPERTY( BlueprintAssignable )
	FCompletionDelegate OnCompleted;

	// Blueprint Async Action API
	void Activate( ) override;

protected:
	// The identifier for the requested fade
	UPROPERTY( )
	FGameplayTag FadeID;

	// The direction to apply the fade
	enum class EDirection
	{
		Out,
		In,
	};

	// The desired direction of the requested fade
	EDirection FadeDirection;
};