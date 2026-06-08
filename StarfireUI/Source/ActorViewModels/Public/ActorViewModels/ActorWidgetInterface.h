
#pragma once

#include "UObject/Interface.h"

#include "ActorWidgetInterface.generated.h"

// Static class for interface that provides an API for widgets strong bound to a single Actor
UINTERFACE( MinimalAPI, BlueprintType, meta = (DisplayName = "Actor Widget Object") )
class UActorWidgetInterface : public UInterface
{
	GENERATED_BODY( )
public:
};

// Actual interface class to derive from implement a widget strongly bound to a single Actor
class ACTORVIEWMODELS_API IActorWidgetInterface
{
	GENERATED_BODY( )
public:
	// Retrieve the actor that the widget is meant to represent
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent )
	[[nodiscard]] AActor* GetActor( ) const;
	[[nodiscard]] virtual AActor* GetActor_Implementation( ) const { return nullptr; }
};