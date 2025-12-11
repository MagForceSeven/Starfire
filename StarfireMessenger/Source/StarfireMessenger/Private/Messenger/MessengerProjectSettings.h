
#pragma once

#include "Engine/DeveloperSettings.h"

#include "MessengerProjectSettings.generated.h"

// Identifier for the underlying type of message a blueprint struct should represent
UENUM( )
enum class EMessageType : uint8
{
	// Fire-and-forget
	Immediate,

	// Broadcast to all listeners, regardless of registration time
	Stateful,
};

// The configuration information for a single blueprint message type
USTRUCT( )
struct FBlueprintMessageConfig
{
	GENERATED_BODY( )
public:
	// The type of message the struct implements
	UPROPERTY( EditDefaultsOnly, Category = "Blueprint Message Config" )
	EMessageType Type = EMessageType::Immediate;

	// The type of context object that should be expected (can be none)
	UPROPERTY( EditDefaultsOnly, Category = "Blueprint Message Config", meta = (DisplayThumbnail = false) )
	TSoftClassPtr< UObject > ContextType;
	
	// A custom name to give to context pins for this type of message
	UPROPERTY( EditDefaultsOnly, Category = "Blueprint Message Config" )
	FText ContextPinName;
};

// Project configuration settings for the Starfire Messenger plugin
UCLASS( Config = "Game", DefaultConfig )
class STARFIREMESSENGER_API UMessengerProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY( )
public:
	// The messenger settings for how to handle certain blueprint structures that are defined in content
	UPROPERTY( EditDefaultsOnly, Config, Category = "Messenger Project Settings", meta = (DisplayThumbnail = false))
	TMap< TSoftObjectPtr< UScriptStruct >, FBlueprintMessageConfig > BlueprintMessageTypes;

	// A collection of message types that should be prevented from being listened for.
	// Similar to how listening for the lowest level message types from this plugin are filtered because it would be unhelpful to get every message ever.
	UPROPERTY( EditDefaultsOnly, Config, Category = "Messenger Project Settings", meta = (DisplayThumbnail = false))
	TArray< TSoftObjectPtr< UScriptStruct > > AdditionalListenExclusionTypes;

	// Customization for the Messenger Pin of custom nodes (if the project wants to use different terminology)
	UPROPERTY( EditDefaultsOnly, Config, Category = "Blueprint" )
	FText MessengerPinNameOverride;

	// Customization of the Blueprint node category where custom node are listed
	UPROPERTY( EditDefaultsOnly, Config, Category = "Blueprint" )
	FText MessengerNodesCategoryOverride;

	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};