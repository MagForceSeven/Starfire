
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFeatureStateChangeObserver.h"
#include "Engine/DeveloperSettings.h"

#include "FeatureContentManager.generated.h"

class UStarfireFeatureData;
class UGameFeaturesSubsystem;

// Simplified status values to wrap the EGameFeaturePluginState values
enum class EPluginStatus
{
	None,
	Installed,
	Registered,
	Loaded,
	Active,
};

// Subsystem for tracking the ownership and active nature of the content package entitlements
UCLASS( )
class STARFIREASSETS_API UFeatureContentManager : public UGameInstanceSubsystem, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY( )
public:
	// Static accessor to the subsystem through a standard WorldContext object
	[[nodiscard]] static UFeatureContentManager* GetSubsystem( const UObject *WorldContext );
	[[nodiscard]] static UFeatureContentManager* GetSubsystem( const UGameInstance *GameInstance );

	// Find a content entitlement by the asset name
	[[nodiscard]] const UStarfireFeatureData* FindFeature( const FString &PluginName ) const;
	[[nodiscard]] TSet< const UStarfireFeatureData* > FindAllFeatures( const TSet< FString > &PluginNames ) const;

	// Check if an entitlement is currently owned (and therefore allowed to be enabled)
	UFUNCTION( BlueprintCallable, Category = "Feature Content" )
	[[nodiscard]] bool IsFeatureOwned( const UStarfireFeatureData *Feature ) const;
	// Check if an entitlement is currently enabled and meant to affect gameplay
	UFUNCTION( BlueprintCallable, Category = "Feature Content" )
	[[nodiscard]] bool IsFeatureEnabled( const UStarfireFeatureData *Feature ) const;

	// Get the collection of entitlements that are currently owned
	UFUNCTION( BlueprintCallable, CustomThunk, Category = "Feature Content" )
	[[nodiscard]] TArray< const UStarfireFeatureData* > GetOwnedFeatures( ) const;
	// Get the primary asset ids for all of the content entitlements that are currently owned
	UFUNCTION( BlueprintCallable, Category = "Feature Content" )
	[[nodiscard]] TArray< FPrimaryAssetId > GetOwnedFeatureIDs( ) const;

	// Get the collection of entitlements that are enabled for affecting gameplay
	UFUNCTION( BlueprintCallable, CustomThunk, Category = "Plugin Content" )
	[[nodiscard]] TArray< const UStarfireFeatureData* > GetEnabledFeatures( ) const;
	// Get the names for the set of entitlements that are enabled for affecting gameplay
	[[nodiscard]] TArray< FString > GetEnabledFeatureNames( void ) const;

	// Get the entire collection of features that could be owned/enabled
	[[nodiscard]] TArray< const UStarfireFeatureData* > GetKnownFeatures( ) const;

	// Turn on an entitlement so that it can affect gameplay
	void EnableFeatures( const TSet< const UStarfireFeatureData* > &ToEnable, const TArray< FName > &Bundles );
	void EnableFeatures( const TSet< FString > &ToEnable, const TArray< FName > &Bundles );
	// Turn off an entitlement so that it will not affect gameplay
	void DisableFeatures( const TSet< const UStarfireFeatureData* > &ToDisable );
	void DisableFeatures( const TSet< FString > &ToDisable );
	// Set the state of the entitlements to a specific collection of entitlements
	void SetEnabledFeatures( const TSet< const UStarfireFeatureData* > &NewFeatures, const TArray< FName > &Bundles );
	void SetEnabledFeatures( const TSet< FString > &NewFeatures, const TArray< FName > &Bundles );

	// Force all the entitlements into the disabled state
	void DisableAllFeatures( void );

	// Specify a collection of features that should be treated as owned by the player
	void SetFeaturesAsOwned( const TSet< const UStarfireFeatureData* > &ToOwn );
	void SetFeaturesAsOwned( const TSet< FString > &ToOwn );
	// Specify a collection of features that should be treated as unowned by the player
	TSet< const UStarfireFeatureData* > SetFeaturesAsUnowned( TSet< const UStarfireFeatureData* > ToDisown, bool bIgnoreBuiltIns = true );
	TSet< const UStarfireFeatureData* > SetFeaturesAsUnowned( const TSet< FString > &ToDisown, bool bIgnoreBuiltIns = true );
	// Force the state of owned features to be a specific collection (returning the collection of currently enabled features that have become unowned)
	TSet< const UStarfireFeatureData* > SetOwnedFeatures( const TSet< const UStarfireFeatureData* > &ToOwn, bool bIgnoreBuiltIns = true );
	TSet< const UStarfireFeatureData* > SetOwnedFeatures( const TSet< FString > &ToOwn, bool bIgnoreBuiltIns = true );
	// Force the state of all features to be unowned (returning the collection of currently enabled features that have become unowned)
	TSet< const UStarfireFeatureData* > DisownAllFeatures( bool bIgnoreBuiltIns = false );

	// Determine if all the specified packages have made it to the active state
	[[nodiscard]] bool AreAllFeaturesActive( const TArray< const UStarfireFeatureData* > &Features ) const;
	[[nodiscard]] bool AreAllFeaturesActive( const TArray< FString > &PluginNames ) const;
	// Determine if all the packages that have been been enabled have made it to the active state
	[[nodiscard]] bool AreEnabledFeaturesActive( void ) const;

	// Check the status of a plugin package
	[[nodiscard]] EPluginStatus GetFeaturePluginStatus( const UStarfireFeatureData *Feature ) const;
	[[nodiscard]] EPluginStatus GetFeaturePluginStatus( const FString &PluginName ) const;

	// Determine what GameFeaturePlugin (if any) an object is a part of
	[[nodiscard]] FString GetObjectFeaturePluginName( const TSoftObjectPtr< UObject > &Content ) const;
	[[nodiscard]] FString GetObjectFeaturePluginName( const FSoftObjectPath &ContentPath ) const;

	// Determine the GameFeature associated with any particular object
	[[nodiscard]] const UStarfireFeatureData* GetObjectFeaturePlugin( const TSoftObjectPtr< UObject > &Content ) const;
	[[nodiscard]] const UStarfireFeatureData* GetObjectFeaturePlugin( const FSoftObjectPath &ContentPath ) const;

#if !UE_BUILD_SHIPPING
	// Enable the features specified by the DeveloperSettings as those that should be enabled for PIE
	void EnableDeveloperPIEFeatures( const UWorld *World, const TArray< FName > &Bundles );
#endif

	// Subsystem API
	void Initialize( FSubsystemCollectionBase &Collection ) override;
	void Deinitialize( void ) override;

	// State Changer Observer API
	void OnGameFeatureRegistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL ) override;
	void OnGameFeatureUnregistering( const UGameFeatureData *GameFeatureData, const FString &PluginName, const FString &PluginURL ) override;
	
	// Delegate type for broadcasting changes in feature state
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FFeatureStateChanged, const UStarfireFeatureData*, FeatureData );

	// The feature content is now owned by the local player
	UPROPERTY( BlueprintReadOnly, BlueprintAssignable, Category = "Feature Content" )
	FFeatureStateChanged OnFeatureOwned;
	// The feature content is no longer owned by the local player
	UPROPERTY( BlueprintReadOnly, BlueprintAssignable, Category = "Feature Content" )
	FFeatureStateChanged OnFeatureDisowned;
	// The feature is being enabled (but the content hasn't been enabled yet)
	UPROPERTY( BlueprintReadOnly, BlueprintAssignable, Category = "Feature Content" )
	FFeatureStateChanged OnFeatureEnabling;
	// The feature is being disabled (it's content is still available)
	UPROPERTY( BlueprintReadOnly, BlueprintAssignable, Category = "Feature Content" )
	FFeatureStateChanged OnFeatureDisabling;

private:
	// The complete set of content packages that are possible to own or enable
	UPROPERTY( )
	TArray< TObjectPtr< const UStarfireFeatureData > > KnownFeatureData;

	// The content packages owned by the user (as defined by the platform or developer settings)
	UPROPERTY( )
	TSet< TObjectPtr< const UStarfireFeatureData > > OwnedFeatures;

	// The content packages that are enabled by the user to actually affect the game
	UPROPERTY( )
	TSet< TObjectPtr< const UStarfireFeatureData > > EnabledFeatures;

	// Direct mapping of the PluginName to the feature data
	UPROPERTY( )
	TMap< FString, TObjectPtr< const UStarfireFeatureData > > DataMapping;

	// Direct mapping of the feature data to the PluginName
	UPROPERTY( )
	TMap< TObjectPtr< const UStarfireFeatureData >, FString > NameMapping;

	// Direct mapping of the feature data to the PluginURL
	UPROPERTY( )
	TMap< TObjectPtr< const UStarfireFeatureData >, FString > URLMapping;

	DECLARE_FUNCTION( execGetOwnedFeatures );
	DECLARE_FUNCTION( execGetEnabledFeatures );
};

// Developer settings that can be used for PIE sessions to configure the ownership of DLC content
UCLASS( Config = "Game" )
class UContentPackageSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:
	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
	// Entitlements that should be treated as owned by the player while debugging in PIE or standalone or any other time
	// an online service may not be available to act as ownership authority
	UPROPERTY( EditAnywhere, Config, Category = "Content Entitlements", meta = (DisplayThumbnail = false) )
	TArray< TSoftObjectPtr< const UStarfireFeatureData > > DebugEntitledFeatures;

	// Built-in entitlements that should be treated as unowned by the player while debugging in PIE or standalone or any other time
	// an online service may not be available to act as ownership authority
	UPROPERTY( EditAnywhere, Config, Category = "Content Entitlements", meta = (DisplayThumbnail = false) )
	TArray< TSoftObjectPtr< const UStarfireFeatureData > > DisabledBuiltInFeatures;

	// The features that should be enabled when starting a game through PIE
	// Features should not be enabled by a main menu, even in PIE. Nor should game started by a main menu
	// even if the menu was started in PIE
	UPROPERTY( EditAnywhere, Config, Category = "Content Entitlements", meta = (DisplayName = "PIE Enabled Features", DisplayThumbnail = false) )
	TArray< TSoftObjectPtr< const UStarfireFeatureData > > PIEEnabledFeatures;

	// Features which shouldn't be enabled, even when starting a game through PIE
	UPROPERTY( EditAnywhere, Config, Category = "Content Entitlements", meta = (DisplayName = "PIE Disabled Features", DisplayThumbnail = false) )
	TArray< TSoftObjectPtr< const UStarfireFeatureData > > PIEDisabledFeatures;
};