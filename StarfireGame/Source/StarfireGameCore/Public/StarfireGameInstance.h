
#pragma once

#include "CommonGameInstance.h"

#include "StarfireGameInstance.generated.h"

// A common implementation of a game instance that includes an integration of the startup for the Data Definition Library
// and DevSettings Preloader interface
UCLASS( )
class STARFIREGAMECORE_API UStarfireGameInstance : public UCommonGameInstance
{
	GENERATED_BODY( )
public:
	// Game Instance API
	void Init( ) override;
	void Shutdown( ) override;
};