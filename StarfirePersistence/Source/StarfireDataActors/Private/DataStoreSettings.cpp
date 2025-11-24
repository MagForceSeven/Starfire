
#include "DataStoreSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataStoreSettings)

#define LOCTEXT_NAMESPACE "StarfireDataActors_DeveloperSettings"

FName UDataStoreSettings::GetContainerName( ) const
{
	return FName( "Project" );
}

FName UDataStoreSettings::GetCategoryName( ) const
{
	return FName( "Game" );
}

FName UDataStoreSettings::GetSectionName( ) const
{
	return FName( "Starfire Data Actors Settings" );
}

#if WITH_EDITOR
FText UDataStoreSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Starfire Data Actors Settings" );
}

FText UDataStoreSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Configuration options for the Starfire Data Actors module." );
}
#endif

#undef LOCTEXT_NAMESPACE