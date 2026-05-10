
#include "StarfireGameUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireGameUserSettings)

UStarfireGameUserSettings* UStarfireGameUserSettings::Get( )
{
	return CastChecked< UStarfireGameUserSettings >( GEngine->GetGameUserSettings( ) );
}