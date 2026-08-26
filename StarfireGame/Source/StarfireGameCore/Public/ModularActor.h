
#pragma once

#include "GameFramework/Actor.h"

#include "ModularActor.generated.h"

// Minimal Actor class that supports extension by game feature plugins
UCLASS( Blueprintable )
class STARFIREGAMECORE_API AModularActor : public AActor
{
	GENERATED_BODY()
public:
	AModularActor( );
};
