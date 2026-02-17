
#include "MessengerPinFactory.h"

#include "SGraphPinMessengerType.h"

#include "K2Node_MessengerNodeBase.h"
#include "K2Node_CastMessage.h"

TSharedPtr< SGraphPin > FMessengerPinFactory::CreatePin( UEdGraphPin *InPin ) const
{
	const auto Node = InPin->GetOwningNode( );

	if (const auto CastNode = Cast< UK2Node_CastMessage >( Node ))
	{
		if (CastNode->GetTypePin( ) == InPin)
			return SNew( SGraphPinMessengerType, InPin, UK2Node_MessengerNodeBase::GetBaseAllowedType( CastNode->GetCastSourcePin( ) ), CastNode->bAllowsAbstract );
	}
	
	if (const auto BaseNode = Cast< UK2Node_MessengerNodeBase >( Node ))
	{
		if (BaseNode->GetTypePin( ) == InPin)
			return SNew( SGraphPinMessengerType, InPin, BaseNode->bAllowImmediate, BaseNode->bAllowStateful, BaseNode->bAllowsAbstract );
	}
	
	return nullptr;
}
