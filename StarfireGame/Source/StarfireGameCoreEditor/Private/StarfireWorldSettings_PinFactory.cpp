
#include "StarfireWorldSettings_PinFactory.h"

#include "SGraphPin_WorldSettingsExtension.h"

#include "StarfireWorldSettings.h"

// Blueprint Graph
#include "K2Node_CallFunction.h"

TSharedPtr< SGraphPin > FStarfireWorldSettings_PinFactory::CreatePin( UEdGraphPin *InPin ) const
{
	const auto Node = InPin->GetOwningNode( );
	
	if (const auto FunctionNode = Cast< UK2Node_CallFunction >( Node ) )
	{
		if (FunctionNode->FunctionReference.GetMemberParentClass( ) == AStarfireWorldSettings::StaticClass( ))
		{
			if (FunctionNode->FunctionReference.GetMemberName( ) == GET_FUNCTION_NAME_CHECKED( AStarfireWorldSettings, FindExtensionByClass_BP ))
			{
				static const FName ParamName( "Type" ); // No helpful macro to catch a compiler error sadly
				if (InPin->PinName == ParamName)
					return SNew( SGraphPin_WorldSettingsExtension, InPin );
			}

			if (FunctionNode->FunctionReference.GetMemberName( ) == GET_FUNCTION_NAME_CHECKED( AStarfireWorldSettings, GetAllExtensionsByClass_BP ))
			{
				static const FName ParamName( "Type" ); // No helpful macro to catch a compiler error sadly
				if (InPin->PinName == ParamName)
					return SNew( SGraphPin_WorldSettingsExtension, InPin );
			}
		}
	}
	
	return nullptr;
}
