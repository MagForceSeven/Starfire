
#pragma once

#include "GameFramework/WorldSettings.h"

#include "NativeGameplayTags_SF.h"
#include "Templates/TypeUtilitiesSF.h"
#include "Templates/SFArrayCaster.h"

#include "StructUtils/StructView.h"
#include "Templates/SubScriptStructOf.h"

#include "StarfireWorldSettings.generated.h"

// Base structure for world setting extensions
USTRUCT( BlueprintType )
struct FStarfireWorldSettingExtension
{
	GENERATED_BODY( )
public:
};

// Concept for limiting template parameters to derived extension structure types
template < class type_t >
concept CWorldSettingExtension = SFstd::derived_from< type_t, FStarfireWorldSettingExtension > && !SFstd::same_as< type_t, FStarfireWorldSettingExtension >;

// A World Settings class that add in an identifier for the type of world that is easier to check against in certain circumstances
UCLASS( )
class STARFIREGAMECORE_API AStarfireWorldSettings : public AWorldSettings
{
	GENERATED_BODY( )
public:
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( WorldType_Root )
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( WorldType_Unknown )

	AStarfireWorldSettings( );

	// Static accessors to the Starfire specific world settings type
	[[nodiscard]] static const AStarfireWorldSettings* GetWorldSettings( const UObject *WorldContext );
	[[nodiscard]] static const AStarfireWorldSettings* GetWorldSettings( const UWorld *World );

	// Native Accessor to the world type
	[[nodiscard]] FGameplayTag GetWorldType( void ) const { return WorldType; }

	// Find the first extension of a specific type affecting this asset
	template< CWorldSettingExtension type_t >
	[[nodiscard]] TConstStructView< type_t > FindExtensionByClass( void ) const;
	[[nodiscard]] TConstStructView< FStarfireWorldSettingExtension > FindExtensionByClass( const TSubScriptStructOf< FStarfireWorldSettingExtension > &Type ) const;

	// Get all the active extensions of a certain type affecting this asset
	template< CWorldSettingExtension type_t >
	[[nodiscard]] TArray< TConstStructView< type_t > > GetAllExtensionsByClass( void ) const;
	[[nodiscard]] TArray< TConstStructView< FStarfireWorldSettingExtension > > GetAllExtensionsByClass( const TSubScriptStructOf< FStarfireWorldSettingExtension > &Type ) const;
	
	// Accessor to all the extensions in one go
	[[nodiscard]] TArray< TConstStructView< FStarfireWorldSettingExtension > > GetAllExtensions( void ) const;

private:
	// An identifier for the logical type of world this is for gameplay purposes
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = World, meta = (Categories="World.Type", AllowPrivateAccess = true) )
	FGameplayTag WorldType;

	// Extra data to be used by this world during gameplay
	UPROPERTY( EditDefaultsOnly, Category = World, meta = (ExcludeBaseStruct))
	TArray< TInstancedStruct< FStarfireWorldSettingExtension > > Extensions;

	friend class FStarfireWorldSettings_PinFactory;

	// Find the first extension of a specific type affecting this World
	UFUNCTION( BlueprintCallable, Category = "World Settings", DisplayName = "Find Extension by Class", meta = (ExcludeBaseStruct, WorldContext = "WorldContext") )
	static FInstancedStruct FindExtensionByClass_BP( const UObject *WorldContext, const UScriptStruct *Type );
	
	// Get all the active extensions of a certain type affecting this World
	UFUNCTION( BlueprintCallable, Category = "World Settings", DisplayName = "Get All Extensions by Class", meta = (ExcludeBaseStruct, WorldContext = "WorldContext") )
	static TArray< FInstancedStruct > GetAllExtensionsByClass_BP( const UObject *WorldContext, const UScriptStruct *Type );
};

//**********************************************************************************************************************
//						Template Implementations

template< CWorldSettingExtension type_t >
TConstStructView< type_t > AStarfireWorldSettings::FindExtensionByClass( void ) const
{
	const auto View = FindExtensionByClass( type_t::StaticStruct( ) );

	TConstStructView< type_t > Result;
	Result.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

	return Result;
}

template< CWorldSettingExtension type_t >
TArray< TConstStructView< type_t > > AStarfireWorldSettings::GetAllExtensionsByClass( void ) const
{
	// Reinterpret the array as a struct view of the derived type which we're guaranteed all elements are compatible with
	return TSFArrayCaster( GetAllExtensionsByClass( type_t::StaticStruct( ) ) ).template Get< TConstStructView< type_t > >( );
}