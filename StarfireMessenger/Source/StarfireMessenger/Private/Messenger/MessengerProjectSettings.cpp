
#include "Messenger/MessengerProjectSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessengerProjectSettings)

#define LOCTEXT_NAMESPACE "MessengerProjectSettings"

FName UMessengerProjectSettings::GetContainerName( ) const
{
	return FName( "Project" );
}

FName UMessengerProjectSettings::GetCategoryName( ) const
{
	return FName( "Game" );
}

FName UMessengerProjectSettings::GetSectionName( ) const
{
	return FName( "Starfire Messenger Settings" );
}

#if WITH_EDITOR
FText UMessengerProjectSettings::GetSectionText( ) const
{
	return LOCTEXT( "SectionText", "Starfire Messenger Settings" );
}

FText UMessengerProjectSettings::GetSectionDescription( ) const
{
	return LOCTEXT( "SectionDescription", "Configuration options for the Starfire Messenger Plugin" );
}
#endif

#undef LOCTEXT_NAMESPACE