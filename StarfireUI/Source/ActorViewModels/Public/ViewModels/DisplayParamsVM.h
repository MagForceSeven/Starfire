
#pragma once

#include "ActorViewModels/ActorVMBase.h"

#include "DisplayParamsVM.generated.h"

// A view model that exposes the data from the Display Params Interface
UCLASS( BlueprintType )
class ACTORVIEWMODELS_API UDisplayParamsVM : public UActorVMBase
{
	GENERATED_BODY( )
public:
	// Get a short description of the object
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	FText DisplayName;

	// Get a pluralized version of the short description
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	FText DisplayName_Plural;

	// Get a long description of the object
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	FText Description;

	// Get a large icon that can be used to represent the Actor
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	TObjectPtr< UTexture2D > Image_Large;

	// Get a small icon that can be used to represent the Actor
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	TObjectPtr< UTexture2D > Image_Small;

	// Get a large icon that can be used to represent the object
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	TSoftObjectPtr< UTexture2D > Image_Large_Soft;

	// Get a large icon that can be used to represent the object
	UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "UI|Display" )
	TSoftObjectPtr< UTexture2D > Image_Small_Soft;

protected:
	// Actor VM Base
	void HandleOnCreate( AActor *Actor ) override;
};