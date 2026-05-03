
#include "Templates/ActorSingletonAccessor.h"

#include "EngineUtils.h"

const UWorld * FActorSingletonAccessorsBase::GetWorldFromContext( const UObject *WorldContext )
{
	if (!IsValid( WorldContext ))
		return nullptr;

	return GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
}

AActor* FActorSingletonAccessorsBase::GetActorSingleton( const UWorld *World, const TSubclassOf< AActor > &SingletonType )
{
	if (!IsValid( World ))
		return nullptr;

	const FActorIterator It( World, SingletonType );
	if (It)
		return *It;

	return nullptr;
}
