
#include "StarfireUIPinFactory.h"

#include "SGraphPin_ScreenClass.h"

#include "StarfireScreen.h"

// Blueprint Graph
#include "K2Node_CallFunction.h"

TSharedPtr< SGraphPin > FStarfireUIPinFactory::CreatePin( UEdGraphPin *InPin ) const
{
	const auto Node = InPin->GetOwningNode( );
	
	if (const auto FunctionNode = Cast< UK2Node_CallFunction >( Node ) )
	{
		if (FunctionNode->FunctionReference.GetMemberParentClass( ) == UStarfireScreen::StaticClass( ))
		{
			if (FunctionNode->FunctionReference.GetMemberName( ) == GET_FUNCTION_NAME_CHECKED( UStarfireScreen, OpenNewScreen ))
			{
				static const FName ParamName( "ScreenType" ); // No helpful macro to catch a compiler error sadly
				if (InPin->PinName == ParamName)
					return SNew( SGraphPin_ScreenClass, InPin );
			}
		}
	}
	
	return nullptr;
}
