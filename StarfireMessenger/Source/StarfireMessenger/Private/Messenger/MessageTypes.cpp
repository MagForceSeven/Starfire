
#include "Messenger/MessageTypes.h"

#include "Messenger/MessengerProjectSettings.h"

// Core UObject
#include "StructUtils/InstancedStruct.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessageTypes)

#if PLATFORM_UNIX
// Linux doesn't like using the pointer hash for function pointers implicitly like windows
// so we'll just declare an explicit one
[[nodiscard]] inline uint32 PointerHash( UScriptStruct*(*Key)(void) )
{
	return PointerHash( (const void*)Key );
}
#endif

TSet< FString > StatefulTypesWithContexts;
TMap< UScriptStruct*(*)(void), UClass*(*)(void) > ContextPinTypes_Pending;
TSet< FString > AbstractTypes;

#if WITH_EDITORONLY_DATA
TMap< FString, const UClass* > ContextPinTypes;
TMap< FString, FText > ContextPinNameOverrides;

TMap< UScriptStruct*(*)(void), FText > ContextPinNameOverrides_Pending;

FMessageContextNameMarker::FMessageContextNameMarker( UScriptStruct* (*StructGetter)(void), const char* ContextName )
{
	ContextPinNameOverrides_Pending.Add( StructGetter, FText::FromString( ContextName ) );
}

#endif

FAbstractMarker::FAbstractMarker( const char* Typename )
{
	const FString Name( Typename );
	AbstractTypes.Add( Name.RightChop( 1 ) );
}

FMessageContextTypeMarker::FMessageContextTypeMarker( UScriptStruct* (*StructGetter)(void), UClass*(*TypeGetter)(void) )
{
	ContextPinTypes_Pending.Add( StructGetter, TypeGetter );
}

bool FSf_MessageBase::IsMessageTypeAbstract( const UScriptStruct *MessageType )
{
	return AbstractTypes.Contains( MessageType->GetName( ) );
}

#if WITH_EDITOR
TSoftClassPtr< UObject > FSf_MessageBase::GetContextType( const UScriptStruct *MessageType )
{
	if (MessageType->IsNative( ))
	{
		if (ensureAlways(MessageType->IsChildOf( FSf_MessageBase::StaticStruct( ) )))
		{
			const UStruct* Type = MessageType;

			// Search up the hierarchy to see if anyone specifies a type for a context pin
			while (Type != nullptr)
			{
				if (const auto Found = ContextPinTypes.Find( Type->GetName( )))
					return *Found;

				Type = Type->GetSuperStruct( );
			}
		}

		return nullptr;
	}

	const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (const auto MessageConfig = Settings->BlueprintMessageTypes.Find( MessageType ))
		return MessageConfig->ContextType;

	return nullptr;
}

FText FSf_MessageBase::GetContextPinName( const UScriptStruct *MessageType )
{
	if (MessageType->IsNative( ))
	{
		const UStruct* Type = MessageType;

		// Search up the hierarchy to see if anyone specifies an override for the pin name
		while (Type != nullptr)
		{
			if (const auto Found = ContextPinNameOverrides.Find( Type->GetName( )))
				return *Found;

			Type = Type->GetSuperStruct( );
		}
	}
	else
	{
		const auto Settings = GetDefault< UMessengerProjectSettings >( );
		if (const auto MessageConfig = Settings->BlueprintMessageTypes.Find( MessageType ))
		{
			if (!MessageConfig->ContextPinName.IsEmpty( ))
				return MessageConfig->ContextPinName;
		}
	}

	return NSLOCTEXT( "StarfireMessenger_Editor", "ContextPin_Default", "Context" );
}

void FSf_MessageBase::RemapMessageContextEditorData( )
{
	for (const auto &[ MessageTypeGetter, Name ] : ContextPinNameOverrides_Pending)
	{
		const UScriptStruct* MessageType = MessageTypeGetter( );
		
		ContextPinNameOverrides.Add( MessageType->GetName( ), Name );
	}
	ContextPinNameOverrides_Pending.Empty( );
}
#endif

void FSf_MessageBase::RemapMessageContextData( )
{
	for (const auto &[ MessageTypeGetter, ContextTypeGetter ] : ContextPinTypes_Pending)
	{
		const UScriptStruct* MessageType = MessageTypeGetter( );
		const UClass* ContextType = ContextTypeGetter( );

		// For the runtime, we only care about stateful messages
		if (MessageType->IsChildOf( FSf_Message_Stateful::StaticStruct( ) ) && (ContextType != nullptr))
			StatefulTypesWithContexts.Add( MessageType->GetName( ) );

#if WITH_EDITOR
		ContextPinTypes.Add( MessageType->GetName( ), ContextType );
#endif
	}
	
	ContextPinTypes_Pending.Empty( );
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

bool FSf_MessageBase::DoesStatefulTypeRequireContext( const UScriptStruct *MessageType )
{
	if (MessageType->IsNative( ))
	{
		if (ensureAlways(MessageType->IsChildOf< FSf_Message_Stateful >( )))
		{
			const UStruct* Type = MessageType;

			// Search up the hierarchy to see if anyone specifies a type for a context pin
			while (Type != nullptr)
			{
				if (StatefulTypesWithContexts.Contains( Type->GetName( ) ))
					return true;

				Type = Type->GetSuperStruct( );
			}
		}

		return false;
	}

	const auto Settings = GetDefault< UMessengerProjectSettings >( );
	if (const auto MessageConfig = Settings->BlueprintMessageTypes.Find( MessageType ))
		return !MessageConfig->ContextType.IsNull( );

	return false;
}
