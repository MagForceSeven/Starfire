
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameplayTagK2Statics.generated.h"

struct FGameplayTag;

UCLASS( )
class UGameplayTagK2Statics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
private:
	friend class UK2Node_HierarchicalGameplayTagSwitch;

	// Utility function for custom gameplay tag switch
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = true))
	static int DetermineBestMatch( FGameplayTag CheckTag, const TArray< FGameplayTag > &Tags );
};