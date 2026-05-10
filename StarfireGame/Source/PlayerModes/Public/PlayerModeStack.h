
#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "Templates/SubsystemNativeAccessors.h"

#include "Templates/TypeUtilitiesSF.h"

#include "PlayerModeStack.generated.h"

DECLARE_STATS_GROUP( TEXT( "PlayerModeStack" ), STATGROUP_PlayerModeStack, STATCAT_Advanced );

class APlayerModeBase;

template < class type_t >
concept CPlayerModeType = SFstd::derived_from< type_t, APlayerModeBase >;

// Collection of player modes that are managed as a stack with the top mode actively affecting the player
UCLASS( )
class PLAYERMODES_API UPlayerModeStack : public ULocalPlayerSubsystem, public FTickableGameObject, public TSubsystemNativeAccessors< UPlayerModeStack >
{
	GENERATED_BODY( )
public:
	// Add a new mode to the top of the stack and make it active
	void PushMode( APlayerModeBase* Mode );

	// Remove the top mode from the stack and re-activate the mode now at the top
	UFUNCTION( BlueprintCallable, Category = "Player Modes" )
	void PopMode( );

	// Remove a mode from anywhere in the stack (removing the top functions the same as a Pop)
	UFUNCTION( BlueprintCallable, Category = "Player Modes" )
	void RemoveMode( APlayerModeBase* Mode );

	// Get the mode that is currently at the top of stack
	UFUNCTION( BlueprintCallable, Category = "Player Modes" )
	APlayerModeBase* GetCurrentMode( ) const;

	DECLARE_MULTICAST_DELEGATE_TwoParams( FPlayerModeStackChange, UPlayerModeStack*, APlayerModeBase* );
	// Hook for responding to new player modes being added to the top of the stack
	FPlayerModeStackChange OnPlayerModePushed;
	// Hook for responding to player modes being removed from the stack
	FPlayerModeStackChange OnPlayerModeRemoved;
	// Hook for responding to player modes being removed from the top of the stack
	FPlayerModeStackChange OnPlayerModePopped;

	// Get the Pawn being used for control of the mode at the top of the stack
	UFUNCTION( BlueprintCallable, Category = "Player Modes" )
	APawn* GetCurrentPawn( ) const;

	// Get the current mode as a specific type
	template < CPlayerModeType type_t >
	type_t* GetCurrentModeAs( void ) const;

	// Get the current mode as a specific type
	UFUNCTION( BlueprintCallable, Category = "Player Modes", meta = (DeterminesOutputType = ModeType) )
	APlayerModeBase* GetCurrentModeAs( TSubclassOf< APlayerModeBase > ModeType ) const;

	// Check if a certain type of mode is somewhere on the stack
	template < CPlayerModeType type_t >
	bool IsModeOnStack( void ) const;

	// Check if a certain type of mode is somewhere on the stack
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Player Modes" )
	bool IsModeOnStack( TSoftClassPtr< APlayerModeBase > ModeClass ) const;

	// Find the first instance (from the top down) of a player mode type on the stack
	template < CPlayerModeType type_t >
	type_t* FindModeOnStack( void ) const;

	// Find the first instance (from the top down) of a player mode type on the stack
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Player Modes", meta = (DeterminesOutputType = ModeType) )
	APlayerModeBase* FindModeOnStack( TSubclassOf< APlayerModeBase > ModeType ) const;

	// Find the first instance (from the top down) of an Actor Component on a Player Mode
	template < SFstd::derived_from< UActorComponent > type_t >
	type_t* FindComponentOnStack( void ) const;

	// Find the first instance (from the top down) of an Actor Component on a Player Mode
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Player Modes", meta = (DeterminesOutputType = "ComponentClass") )
	UActorComponent* FindComponentOnStack( TSubclassOf< UActorComponent > ComponentClass ) const;

	// Subsystem API
	bool ShouldCreateSubsystem( UObject *Outer ) const override;
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( ) override;

	// Tickable Game Object API
	ETickableTickType GetTickableTickType( ) const override;
	void Tick( float DeltaTime ) override;
	bool IsTickableWhenPaused() const override { return true; }
	UWorld* GetTickableGameObjectWorld( void ) const override;
	TStatId GetStatId( ) const override { RETURN_QUICK_DECLARE_CYCLE_STAT( UPlayerModeStack, STATGROUP_PlayerModeStack ); }

protected:
	// Hook for responding to a new player mode being pushed to the top of the stack
	virtual void HandlePlayerModePushed( APlayerModeBase* PlayerMode ) { }
	// Hook for responding to a player mode being removed from the stack
	virtual void HandlePlayerModeRemoved( APlayerModeBase* PlayerMode ) { }
	// Hook for responding to a player mode being removed from the top of the stack
	virtual void HandlePlayerModePopped( APlayerModeBase* PlayerMode ) { }

private:
	// Collection of actors that make up the stack of active player modes
	UPROPERTY( VisibleInstanceOnly, Category = "Player Modes" )
	TArray< TObjectPtr< APlayerModeBase > > PlayerModeStack;

	// Modes that have been removed from the stack but are still performing cleanup actions prior to being destroyed
	UPROPERTY( VisibleInstanceOnly, Category = "Player Modes" )
	TArray< TObjectPtr< APlayerModeBase > > DeferredCleanup;

	// "mutex" to prevent stack modifications
	UPROPERTY( VisibleInstanceOnly, Category = "Player Modes" )
	bool bStackIsLocked = false;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FPlayerModeStackChange_BP, UPlayerModeStack*, Stack, APlayerModeBase*, Mode );
	// Hook for responding to a new player mode being pushed to the top of the stack
	UPROPERTY( BlueprintAssignable, DisplayName = "On Player Mode Pushed" );
	FPlayerModeStackChange_BP OnPlayerModePushed_BP;
	// Hook for responding to a player mode being removed from the stack
	UPROPERTY( BlueprintAssignable, DisplayName = "On Player Mode Removed" )
	FPlayerModeStackChange_BP OnPlayerModeRemoved_BP;
	// Hook for responding to a player mode being removed from the top of the stack
	UPROPERTY( BlueprintAssignable, DisplayName = "On Player Mode Popped" )
	FPlayerModeStackChange_BP OnPlayerModePopped_BP;

	// Has this subsystem been initialized
	bool bIsInitialized = false;

	// The pawn created for this player by the Engine that should be used until a player mode creates a pawn to use
	UPROPERTY( VisibleInstanceOnly, Category = "Player Modes" )
	TObjectPtr< APawn > DefaultPawn;

	// **********************************************************************************************************
	// Custom Blueprint Node Support
	friend class UK2Node_PushPlayerMode;

	DECLARE_DYNAMIC_DELEGATE_OneParam( FModePushCallback, APlayerModeBase*, PlayerMode );

	UFUNCTION( BlueprintCallable, meta = (BlueprintInternalUseOnly = true) )
	void PushModeSoft( TSoftClassPtr< APlayerModeBase > ModeClass, const FModePushCallback &PrePush, const FModePushCallback &PostPush, const FModePushCallback &OnError );
};

#if CPP
#define PLAYER_MODE_STACK_HPP
#include "../Private/PlayerModeStack.hpp"
#undef PLAYER_MODE_STACK_HPP
#endif