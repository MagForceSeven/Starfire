
#pragma once

#include "GameFramework/WorldSettings.h"

#include "NativeGameplayTags_SF.h"

#include "StarfireWorldSettings.generated.h"

// A World Settings class that add in an identifier for the type of world that is easier to check against in certain circumstances
UCLASS( )
class STARFIREGAMECORE_API AStarfireWorldSettings : public AWorldSettings
{
	GENERATED_BODY( )
public:
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( WorldType_Root )
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( WorldType_Unknown )

	AStarfireWorldSettings( );

	// Native Accessor to the world type
	FGameplayTag GetWorldType( void ) const { return WorldType; }

private:
	// An identifier for the logical type of world this is for gameplay purposes
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = World, meta = (Categories="World.Type", AllowPrivateAccess = true) )
	FGameplayTag WorldType;
};