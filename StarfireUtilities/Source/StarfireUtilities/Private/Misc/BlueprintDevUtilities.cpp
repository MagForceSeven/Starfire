
#include "Misc/BlueprintDevUtilities.h"

void UBlueprintDevUtilities::Ensure( bool Condition, const FString &Message )
{
	if (Message.IsEmpty( ))
		ensureAlwaysMsgf( Condition, TEXT( "An ensure was triggered in a blueprint." ) );
	else
		ensureAlwaysMsgf( Condition, TEXT( "%s" ), *Message );
}

void UBlueprintDevUtilities::Check( bool Condition, const FString &Message )
{
	check( Condition );
}

EBuildType UBlueprintDevUtilities::GetCurrentBuildType( )
{
#if UE_BUILD_SHIPPING
	return EBuildType::Shipping;
#elif UE_BUILD_TEST
	return EBuildType::Testing;
#else
	return EBuildType::Development;
#endif
}

void UBlueprintDevUtilities::SwitchOnBuildType( EBuildType &Exec )
{
#if UE_BUILD_SHIPPING
	Exec = EBuildType::Shipping;
#elif UE_BUILD_TEST
	Exec = EBuildType::Testing;
#else
	Exec = EBuildType::Development;
#endif
}