
#include "Messenger/MessageProperty.h"

#include "Messenger/MessageTypes.h"
#include "Messenger/MessengerProjectSettings.h"

#include "Lambdas/InvokedScope.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessageProperty)

bool FStarfireMessageType::IsValid( FString *ErrorMsg ) const
{
	static const auto Settings = GetDefault< UMessengerProjectSettings >( );
	check( Settings != nullptr );

	static const TArray< const UScriptStruct* > IgnoredTypes = INVOKED_SCOPE
	{
		TArray< const UScriptStruct* > Ignored = { FSf_MessageBase::StaticStruct( ), FSf_Message_Immediate::StaticStruct( ), FSf_Message_Stateful::StaticStruct( ) };
		
		for (const auto& Type : Settings->AdditionalListenExclusionTypes)
			Ignored.Push( Type.Get( ) );
			
		return Ignored;
	};

	if (MessageType == nullptr)
	{
		if (ErrorMsg != nullptr)
			*ErrorMsg = TEXT("Message type is not set.");

		return false;
	}
	
	const auto bIsAbstract = FSf_MessageBase::IsMessageTypeAbstract( MessageType );
	if (bIsAbstract && !bAllowAbstract)
	{
		if (ErrorMsg != nullptr)
			*ErrorMsg = TEXT("Abstract Message Types not allowed.");

		return false;
	}

	if (MessageType->IsNative( ))
	{
		if (IgnoredTypes.Contains( MessageType ))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Selected Message Type should not be pickable.");

			return false;
		}

		if (!MessageType->IsChildOf< FSf_MessageBase >( ))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Selected structure type is not a Message Type.");

			return false;
		}
		
		if ((CustomBaseType != nullptr) && !MessageType->IsChildOf( CustomBaseType ))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = FString::Printf( TEXT("Selected Message Type is not a child of required base message type '%s'."), *CustomBaseType->GetDisplayNameText( ).ToString( ));

			return false;
		}

		if (!bAllowStateful && MessageType->IsChildOf< FSf_Message_Stateful >( ))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Stateful messages are not pickable in this context.");

			return false;
		}

		if (!bAllowImmediate && MessageType->IsChildOf< FSf_Message_Immediate >( ))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Immediate messages are not pickable in this context.");

			return false;
		}
	}
	else
	{
		const auto Found = Settings->BlueprintMessageTypes.Find( MessageType );
		if (Found == nullptr)
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Selected blueprint structure is not configured as a message type. Update Starfire Messenger project settings.");

			return false;
		}

		if (CustomBaseType != nullptr)
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = FString::Printf( TEXT("Blueprint Message Types not allowed when base message type '%s' is required."), *CustomBaseType->GetDisplayNameText( ).ToString( ));

			return false;
		}

		if (!bAllowStateful && (Found->Type == EMessageType::Stateful))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Stateful messages are not pickable in this context.");

			return false;
		}

		if (!bAllowImmediate && (Found->Type == EMessageType::Immediate))
		{
			if (ErrorMsg != nullptr)
				*ErrorMsg = TEXT("Immediate messages are not pickable in this context.");

			return false;
		}
	}

	return true;
}