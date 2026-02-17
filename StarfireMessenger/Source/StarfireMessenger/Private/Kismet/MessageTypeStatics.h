
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "MessageTypeStatics.generated.h"

struct FInstancedStruct;

UCLASS( )
class UMessageTypeStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
private:
	friend class UK2Node_SwitchMessageType;
	
	// Utility function for custom gameplay tag switch
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = true))
	static int DetermineBestMatch( const FInstancedStruct& CheckInstance, const TArray< UScriptStruct* > &Types );
};