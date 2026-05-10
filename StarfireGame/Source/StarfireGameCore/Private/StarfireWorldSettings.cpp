
#include "StarfireWorldSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireWorldSettings)

UE_DEFINE_GAMEPLAY_TAG_COMMENT( AStarfireWorldSettings::WorldType_Root, "World.Type", "The root for tags to identify different types of game worlds." );
UE_DEFINE_GAMEPLAY_TAG_COMMENT( AStarfireWorldSettings::WorldType_Unknown, "World.Type.Unknown", "Default WorldType for Worlds not yet configured with a type." );

AStarfireWorldSettings::AStarfireWorldSettings( )
{
	WorldType = WorldType_Unknown;

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}