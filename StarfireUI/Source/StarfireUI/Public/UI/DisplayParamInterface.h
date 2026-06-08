
#pragma once

#include "UObject/Interface.h"

#include "DisplayParamInterface.generated.h"

// Static class for interface that provides an API for display information
UINTERFACE( MinimalAPI, BlueprintType, meta = (DisplayName = "Display Param Object", CannotImplementInterfaceInBlueprint) )
class UDisplayParamInterface : public UInterface
{
	GENERATED_BODY( )
public:
};

// Actual interface class to derive from to provide an API for display information
class STARFIREUI_API IDisplayParamInterface
{
	GENERATED_BODY( )
public:
	// Get a short description of the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Display Name") )
	[[nodiscard]] virtual FText GetUIDisplayName( ) const = 0;

	// Get a pluralized version of the short description
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Display Name (Plural)") )
	[[nodiscard]] virtual FText GetUIDisplayName_Plural( ) const { return GetUIDisplayName( ); }

	// Get a long description of the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Description") )
	[[nodiscard]] virtual FText GetUIDescription( ) const { return FText( ); }

	// Get a large icon that can be used to represent the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Image (Large)") )
	[[nodiscard]] virtual UTexture2D* GetUIImage_Large( ) const { return nullptr; }

	// Get a small icon that can be used to represent the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Image (Small)") )
	[[nodiscard]] virtual UTexture2D* GetUIImage_Small( ) const { return nullptr; }

	// Get a soft reference to a large icon that can be used to represent the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Image (Large) (Soft)") )
	[[nodiscard]] virtual TSoftObjectPtr< UTexture2D > GetUIImage_Large_Soft( ) const { return { }; }

	// Get a soft reference to a small icon that can be used to represent the object
	UFUNCTION( BlueprintCallable, Category = "UI|Display", meta = (DisplayName = "Get UI Image (Small) (Soft)") )
	[[nodiscard]] virtual TSoftObjectPtr< UTexture2D > GetUIImage_Small_Soft( ) const { return { }; }
};

#define DISPLAY_PARAM_ROUTE_NAME_TO_DEFINITION( )			[[nodiscard]] FText GetUIDisplayName( ) const override { return GetDefinition( )->GetUIDisplayName( ); }
#define DISPLAY_PARAM_ROUTE_PLURAL_TO_DEFINITION( )			[[nodiscard]] FText GetUIDisplayName_Plural( ) const override { return GetDefinition( )->GetUIDisplayName_Plural( ); }
#define DISPLAY_PARAM_ROUTE_DESCRIPTION_TO_DEFINITION( )	[[nodiscard]] FText GetUIDescription( ) const override { return GetDefinition( )->GetUIDescription( ); }
#define DISPLAY_PARAM_ROUTE_LARGE_TO_DEFINITION( )			[[nodiscard]] UTexture2D* GetUIImage_Large( ) const override { return GetDefinition( )->GetUIImage_Large( ); }
#define DISPLAY_PARAM_ROUTE_SMALL_TO_DEFINITION( )			[[nodiscard]] UTexture2D* GetUIImage_Small( ) const override { return GetDefinition( )->GetUIImage_Small( ); }
#define DISPLAY_PARAM_ROUTE_LARGE_SOFT_TO_DEFINITION( )		[[nodiscard]] UTexture2D* GetUIImage_Large_Soft( ) const override { return GetDefinition( )->GetUIImage_Large_Soft( ); }
#define DISPLAY_PARAM_ROUTE_SMALL_SOFT_TO_DEFINITION( )		[[nodiscard]] UTexture2D* GetUIImage_Small_Soft( ) const override { return GetDefinition( )->GetUIImage_Small_Soft( ); }


#define DISPLAY_PARAM_ROUTE_ALL_TO_DEFINITION( )	\
DISPLAY_PARAM_ROUTE_NAME_TO_DEFINITION( )			\
DISPLAY_PARAM_ROUTE_PLURAL_TO_DEFINITION( )			\
DISPLAY_PARAM_ROUTE_DESCRIPTION_TO_DEFINITION( )	\
DISPLAY_PARAM_ROUTE_LARGE_TO_DEFINITION( )			\
DISPLAY_PARAM_ROUTE_SMALL_TO_DEFINITION( )			\
DISPLAY_PARAM_ROUTE_LARGE_SOFT_TO_DEFINITION( )		\
DISPLAY_PARAM_ROUTE_SMALL_SOFT_TO_DEFINITION( )