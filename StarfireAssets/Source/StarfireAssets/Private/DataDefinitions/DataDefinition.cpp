
#include "DataDefinitions/DataDefinition.h"

#include "DataDefinitions/DefinitionExtension.h"

#include "Debug/Redscreen.h"
#include "Kismet/BlueprintUtilitiesSF.h"

// CoreUObject
#include "UObject/Class.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DataDefinition)

FPrimaryAssetId UDataDefinition::GetPrimaryAssetId( ) const
{
	return FPrimaryAssetId( UDataDefinition::StaticClass( )->GetFName( ), GetFName( ) );
}

const UDataDefinitionExtension* UDataDefinition::FindExtensionByClass( const TSubclassOf< UDataDefinitionExtension > &Type ) const
{
	if (!ensureAlways( Type != nullptr ))
		return nullptr;

	for (const auto &Ext : ActiveExtensions)
	{
		if (Ext->IsA( Type ))
			return Ext;
	}
	
	return nullptr;
}

TArray< const UDataDefinitionExtension* > UDataDefinition::GetAllExtensionsByClass( const TSubclassOf< UDataDefinitionExtension > &Type ) const
{
	TArray< const UDataDefinitionExtension* > Extensions;

	AppendAllExtensionsByClass( Type, Extensions, /*bIncludeDuplicates*/ true );

	return Extensions;
}

void UDataDefinition::AppendAllExtensionsByClass( const TSubclassOf< UDataDefinitionExtension > &Type, TArray< const UDataDefinitionExtension* > &OutExtensions, bool bIncludeDuplicates ) const
{
	if (!ensureAlways( Type != nullptr ))
		return;

	for (const auto &Ext : ActiveExtensions)
	{
		if (Ext->IsA( Type ))
		{
			if (bIncludeDuplicates)
				OutExtensions.Push( Ext );
			else
				OutExtensions.AddUnique( Ext );
		}
	}
}

const UDataDefinitionExtension* UDataDefinition::FindExtension_BP( TSubclassOf< UDataDefinitionExtension > Type, ExecEnum_Validity &Exec, const UObject *WorldContext ) const
{
	Exec = ExecEnum_Validity::Invalid;
	
	if (Type == nullptr)
	{
		Redscreen( WorldContext, TEXT( "DataDefinition::FindExtensionByClass called without specifying an Exention Type %s.%s" ), *WorldContext->GetName( ), *GetBlueprintCallerFunctionName( ) );
		
		return nullptr;
	}

	const auto Extension = FindExtensionByClass( Type );
	if (Extension == nullptr)
		return nullptr;

	Exec = ExecEnum_Validity::Valid;
	return Extension;
}

void UDataDefinition::GetAllExtensions_BP( TSubclassOf< UDataDefinitionExtension > Type, TArray< UDataDefinitionExtension* > &Extensions, const UObject *WorldContext ) const
{
	if (Type == nullptr)
	{
		Redscreen( WorldContext, TEXT( "DataDefinition::GetAllExtensionsByClass called without specifying an Exention Type %s.%s" ), *WorldContext->GetName( ), *GetBlueprintCallerFunctionName( ) );
		
		return;
	}

	AppendAllExtensionsByClass( Type, NativeCompatibilityCast( Extensions ) );
}