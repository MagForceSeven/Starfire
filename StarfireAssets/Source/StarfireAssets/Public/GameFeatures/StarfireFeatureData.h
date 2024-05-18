
#pragma once

#include "GameFeatureData.h"
#include "AssetValidation/VerifiableContent.h"

#include "NativeGameplayTags_SF.h"

// Gameplay Tags
#include "GameplayTagContainer.h"

#include "StarfireFeatureData.generated.h"

// Asset for additional information regarding content packages
UCLASS( )
class STARFIREASSETS_API UStarfireFeatureData : public UGameFeatureData, public IVerifiableAsset
{
	GENERATED_BODY( )
public:
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( ContentType_Root );
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( ContentFlag_Root );
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( ContentFlag_BuiltIn );
	UE_DECLARE_GAMEPLAY_TAG_SCOPED( ContentFlag_Development );
	
	// Player-facing name of the package
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Content Data" )
	FText DisplayName;

	// A shorter player facing name of the package
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Content Data" )
	FText ShortName;

	// Player-facing long description of the package
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Content Data", meta = (MultiLine = true) )
	FText Description;

	// Identifier for an overall category of content represented by this package (dlc, expansion, game mode, etc)
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Content Data", meta = (Categories = "Content.Type") )
	FGameplayTag ContentType;

	// Misc flags regarding how to treat this bundle of content
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Content Data", meta = (Categories = "Content.Flag") )
	FGameplayTagContainer ContentFlags;

	// Verifiable Asset API
	void Verify( const UObject *WorldContext ) override;

	// Object API
	void PostLoad( ) override;
	FPrimaryAssetId GetPrimaryAssetId( ) const override;
};