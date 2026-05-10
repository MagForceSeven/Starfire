
#pragma once

#include "GameFramework/Actor.h"

#include "PlayerModeBase.generated.h"

struct FStreamableHandle;
class UPlayerModeStack;

// States for tracking the state of the player mode
UENUM( )
enum class EPlayerModeState : uint8
{
	Constructed,	// Initial construction state
	Loading,		// Assets have been requested
	Ready,			// Ready to be made active
	CleaningUp,		// Actively cleaning up
	Finished,		// Cleaning process is completed
};

// Base class for actors to control the way the player interacts with the game world (behind the hud)
UCLASS( Abstract )
class PLAYERMODES_API APlayerModeBase : public AActor
{
	GENERATED_BODY( )
public:
	APlayerModeBase( );

	// Accessor for the player controller associated with the player the modes will affect
	UFUNCTION( BlueprintCallable, Category = "Player Mode" )
	[[nodiscard]] APlayerController* GetPlayerController( ) const;

	// Accessor for the pawn that may have been spawned to manage control
	[[nodiscard]] APawn* GetPawn( void ) const { return Pawn; }

protected:
	// Remove this mode from its associated player mode stack subsystem
	UFUNCTION( BlueprintCallable )
	void PopMode( );

	// Function that should be called by modes doing additional cleanup after deactivation when that cleanup is finished
	UFUNCTION( BlueprintCallable )
	void FinishCleaningUp( );

	// Hook for this mode being added to a player mode stack
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnBeginMode( );
	virtual void OnBeginMode_Implementation( ) { }

	// Hook for this mode becoming the active top of a player mode stack (required assets have been loaded) the first time
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnActivate( APlayerModeBase *PreviousMode );
	virtual void OnActivate_Implementation( APlayerModeBase *PreviousMode ) { }

	// Hook for this mode remaining on the stack, but no longer being the active top mode
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnPause( APlayerModeBase *NewMode );
	virtual void OnPause_Implementation( APlayerModeBase *NewMode ) { }

	// Hook for this mode returning to be the top of its associated player mode stack
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnResume( APlayerModeBase *PreviousMode );
	virtual void OnResume_Implementation( APlayerModeBase *PreviousMode ) { }

	// Hook for this mode having been removed from its associated player mode stack and starting its cleanup
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnDeactivate( );
	virtual void OnDeactivate_Implementation( ) { }

	// Hook for this mode about to be destroyed (post deferred cleanup if it required that)
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnEndMode( );
	virtual void OnEndMode_Implementation( ) { }

	// Hook for this mode to determine where its associated pawn should be spawned in the world when it becomes active
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	[[nodiscard]] FTransform GetPawnStartTransform( ) const;
	[[nodiscard]] virtual FTransform GetPawnStartTransform_Implementation( ) const;
	
	// Hook for this mode to respond to the creation of its pawn (prior to its BeginPlay or becoming possessed)
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void OnPawnCreated( APawn *NewPawn );
	virtual void OnPawnCreated_Implementation( APawn *NewPawn ) { }

	// Check if this player mode is ready to become Active
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	[[nodiscard]] bool IsReady( ) const;
	[[nodiscard]] virtual bool IsReady_Implementation( ) const;

	// Hook for derived types to provide a collection of resources that should be loaded asynchronously prior to activation
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	void GatherSoftDependencies( TArray< TSoftObjectPtr< UObject > > &OutDependencies );
	virtual void GatherSoftDependencies_Implementation( TArray< TSoftObjectPtr< UObject > > &OutDependencies );

	// (Optional) The type of pawn that should be created for this type of player mode
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly )
	TSoftClassPtr< APawn > PawnClass;

	// (Optional) A tag that identifies a PlayerStart actor where the pawn should be created
	UPROPERTY( BlueprintReadOnly, EditDefaultsOnly )
	FName PlayerStartTag;
	
	// Whether this type of player mode requires additional, latent, cleanup logic after being deactivated
	UPROPERTY( EditDefaultsOnly )
	bool bRequiresCleanup = false;

private:
	friend class UPlayerModeStack;

	// Update the state of the player mode
	void ChangeState( EPlayerModeState NewState );

	// Update the player mode now having being associated with a player mode stack
	void BeginMode( void );

	// Update the player mode for being the new top of the stack (for the first time)
	void Activate( APlayerModeBase *PreviousMode );

	// Update the player mode for no longer being the top, but still being on the stack
	void Pause( APlayerModeBase *NewMode );

	// Update the player mode for re-becoming the active mode at the top of the stack
	void Resume( APlayerModeBase *PreviousMode );

	// Update the player mode for being removed from the stack and starting its cleanup operations
	void Deactivate( void );

	// Update the player mode for being disassociated with the stack and about to be destroyed
	void EndMode( void );

	// (Optionally) Spawn a pawn that should be used to adjust the control used by the player
	APawn* CreatePawn( APlayerController *Controller );

	// Stop loading or release the resources that may have been requested by this player mode
	void CancelAssetLoading( void );

	// Whether this player mode is actively trying to perform clean up operations
	[[nodiscard]] bool IsCleaningUp( ) const { return State == EPlayerModeState::CleaningUp; }

	// The associated stack that this mode is a part of
	UPROPERTY( VisibleInstanceOnly, meta = (DisplayThumbnail = false) )
	TObjectPtr< UPlayerModeStack  > ActiveStack;

	// The handle for the resources that this player mode requested prior to becoming active
	TSharedPtr< FStreamableHandle > AssetsHandle;

	// The active state of the player mode
	EPlayerModeState State = EPlayerModeState::Constructed;

	// This player mode has been activated after loading dependent resources
	bool bIsActive = false;
	// This player mode was paused by another mode being pushed on top of it
	bool bIsPaused = false;

	// The Pawn that was spawned and possed by this player mode when it was made active
	UPROPERTY( BlueprintReadOnly, VisibleInstanceOnly, meta = (AllowPrivateAccess = true) )
	TObjectPtr< APawn > Pawn;

	// If this mode created it's pawn or reused the pawn from the top of the stack
	UPROPERTY( VisibleInstanceOnly )
	bool bOwnsPawn = false;

#if WITH_EDITOR
public:
	// The names of blueprint events that should be the default (but disabled) set of blueprint nodes on the graph
	static TArray< FName > GetDefaultEventNames( );
#endif
};