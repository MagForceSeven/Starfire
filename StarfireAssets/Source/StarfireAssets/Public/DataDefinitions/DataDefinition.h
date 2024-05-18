
#pragma once

#include "Engine/DataAsset.h"
#include "AssetValidation/VerifiableContent.h"

#include "Templates/TypeUtilitiesSF.h"

#include "DataDefinition.generated.h"

class UDataDefinitionExtension;

class UTexture2D;

enum class ExecEnum_Success : uint8;

// Utility for limiting template parameters to types derived from Definition Extensions
template < class type_t >
concept CExtensionType = SFstd::derived_from< type_t, UDataDefinitionExtension >;

// Base type for game wide static data
UCLASS( Abstract, Blueprintable )
class STARFIREASSETS_API UDataDefinition : public UPrimaryDataAsset, public IVerifiableAsset
{
	GENERATED_BODY( )
public:
	// Object API
	FPrimaryAssetId GetPrimaryAssetId( ) const override;

	// All the extensions that are currently affecting this definition
	UPROPERTY( Transient, BlueprintReadOnly, Category = "Data Definition" )
	TArray< const UDataDefinitionExtension* > ActiveExtensions;

	// Find the first extension of a specific type affecting this asset
	template< CExtensionType type_t >
	const type_t* FindExtensionByClass( void ) const;
	const UDataDefinitionExtension* FindExtensionByClass( const TSubclassOf< UDataDefinitionExtension > &Type ) const;

	// Get all the active extensions of a certain type affecting this asset
	template< CExtensionType type_t >
	TArray< const type_t* > GetAllExtensionsByClass( void ) const;
	TArray< const UDataDefinitionExtension* > GetAllExtensionsByClass( const TSubclassOf< UDataDefinitionExtension > &Type ) const;

	// Add all the active extensions of a certain type to a pre-existing collections
	template< CExtensionType type_t >
	void AppendAllExtensionsByClass( TArray< const type_t* > &OutExtensions, bool bIncludeDuplicates = true ) const;
	void AppendAllExtensionsByClass( const TSubclassOf< UDataDefinitionExtension > &Type, TArray< const UDataDefinitionExtension* > &OutExtensions, bool bIncludeDuplicates = true ) const;

#if WITH_EDITOR
	// Function called by the thumbnail renderer for possibly showing an icon in the content browser
	UE_NODISCARD virtual const UTexture2D* GetThumbnail( ) const { return nullptr; }
#endif

protected:
	friend class UDataDefinitionLibrary;
	
	// Hooks for derived types to respond individually when assets are loaded
	virtual void OnAssetLoaded( void ) const { }
	virtual void OnAssetUnloaded( void ) const { }

private:
	// Find an extension of a specific type
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Data Definition|Extensions", meta = (DeterminesOutputType = "Type", DisplayName = "Find Extension By Class", ExpandEnumAsExecs = "Exec", OutputsAfterExec = "Valid", WorldContext = "WorldContext") )
	const UDataDefinitionExtension* FindExtension_BP( TSubclassOf< UDataDefinitionExtension > Type, ExecEnum_Validity &Exec, const UObject *WorldContext ) const;

	// Get all the extensions of a certain type for this asset
	UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Data Definition|Extensions", meta = (DeterminesOutputType = "Type", DynamicOutputParam = "Extensions", DisplayName = "Get All Extensions By Class", WorldContext = "WorldContext") )
	void GetAllExtensions_BP( TSubclassOf< UDataDefinitionExtension > Type, TArray< UDataDefinitionExtension* > &Extensions, const UObject *WorldContext ) const;
};

// Utility concept for templates needing to restrict to Data Definition subtypes
template < class type_t >
concept CDefinitionType = SFstd::derived_from< type_t, UDataDefinition >;

#if CPP
#define DATA_DEFINITION_HPP
#include "../../Private/DataDefinitions/DataDefinition.hpp"
#undef DATA_DEFINITION_HPP
#endif