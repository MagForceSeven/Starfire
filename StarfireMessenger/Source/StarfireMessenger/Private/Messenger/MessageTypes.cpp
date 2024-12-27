
#include "Messenger/MessageTypes.h"

#include "Messenger/MessengerProjectSettings.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

#if WITH_EDITORONLY_DATA
TSet< FString > AbstractTypes;

FAbstractMarker::FAbstractMarker( const char* Typename )
{
	const FString Name( Typename );
	AbstractTypes.Add( Name.RightChop( 1 ) );
}

bool FSf_MessageBase::IsMessageTypeAbstract( const UScriptStruct *MessageType )
{
	return AbstractTypes.Contains( MessageType->GetName( ) );
}

#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessageTypes)

TSoftClassPtr< UObject > FSf_MessageBase::GetContextType( const UScriptStruct *MessageType )
{
	if (MessageType->IsNative( ))
	{
		if (ensureAlways(MessageType->IsChildOf( FSf_MessageBase::StaticStruct( ) )))
		{
			const FInstancedStruct StructCDO( MessageType );
			const FSf_MessageBase *BasePtr = StructCDO.GetPtr< FSf_MessageBase >( );
			check( BasePtr != nullptr );

			return BasePtr->GetContextType();
		}

		return nullptr;
	}

	const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (const auto MessageConfig = Settings->BlueprintMessageTypes.Find( MessageType ))
		return MessageConfig->ContextType;

	return nullptr;
}

bool FSf_MessageBase::IsMessageTypeStateful( const UScriptStruct *MessageType )
{
	if (MessageType->IsNative( ))
		return MessageType->IsChildOf( FSf_Message_Stateful::StaticStruct( ) );

	const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (const auto MessageConfig = Settings->BlueprintMessageTypes.Find( MessageType ))
		return MessageConfig->Type == EMessageType::Stateful;

	return false;
}
