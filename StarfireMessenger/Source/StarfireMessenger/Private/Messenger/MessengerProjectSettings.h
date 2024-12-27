
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

	// Developer Settings API
	FName GetContainerName( ) const override;
	FName GetCategoryName( ) const override;
	FName GetSectionName( ) const override;

#if WITH_EDITOR
	FText GetSectionText( ) const override;
	FText GetSectionDescription( ) const override;
#endif
};