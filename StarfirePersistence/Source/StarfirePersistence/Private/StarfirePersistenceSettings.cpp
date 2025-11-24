
#include "StarfirePersistenceSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfirePersistenceSettings)

#define LOCTEXT_NAMESPACE "GameSaveData_DeveloperSettings"

bool UStarfirePersistenceSettings::ShouldPersistType( const UClass *ComponentType ) const
{
	for (const auto &PersistType : ComponentsToPersist)
	{
		const auto ComponentClass = PersistType.Get( );
		if (ComponentClass == nullptr)
			continue; // if the class isn't loaded, there can't be an instance of it anywhere, so it doesn't matter

		if (ComponentType->IsChildOf( ComponentClass ))
			return true;
	}

	return false;
}

FName UStarfirePersistenceSettings::GetContainerName( ) const
{
	return FName( "Project" );
}

FName UStarfirePersistenceSettings::GetCategoryName( ) const
{
	return FName( "Game" );
}

FName UStarfirePersistenceSettings::GetSectionName( ) const
{
	return FName( "Starfire Persistence" );
}

#if WITH_EDITOR
FText UStarfirePersistenceSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Starfire Persistence" );
}

FText UStarfirePersistenceSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Configuration settings for the Starfire Persistence plugin" );
}
#endif

#undef LOCTEXT_NAMESPACE