
#include "Messenger/Messenger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Messenger)

//
struct FMessageListener
{
	//
	FMessageListenerHandle Handle;

	//
	TObjectPtr< const UScriptStruct > MessageType;

	//
	TFunction< void( FConstStructView, UObject* )> ImmediateCallback;
	TFunction< void( FConstStructView, UObject*, EStatefulMessageEvent )> StatefulCallback;

	//
	TObjectPtr< const UObject > ContextFilter;
	//
	TObjectPtr< const UObject > Owner;
};

//
struct FStatefulMessages
{
	//
	TMap< TObjectPtr< UObject >, FInstancedStruct > ContextualMessages;

	//
	FInstancedStruct DefaultMessage;
};

[[nodiscard]] static bool DoContextsMatch( const UObject *MessageContext, const UObject *ListenerContext )
{
	if (ListenerContext == nullptr)
		return true;

	if (MessageContext == ListenerContext)
		return true;

	return false;
}

UStarfireMessenger* UStarfireMessenger::GetSubsystem( const UObject *WorldContext )
{
	const auto World = GEngine->GetWorldFromContextObject( WorldContext, EGetWorldErrorMode::LogAndReturnNull );
	if (!ensureAlways( World ))
		return nullptr;

	const auto Subsystem = World->GetSubsystem< UStarfireMessenger >( );
	ensureAlways( Subsystem != nullptr );

	return Subsystem;
}

[[nodiscard]] static bool ShouldBroadcastType( const UStruct *Type )
{
	if (Type == nullptr)
		return false;
	
	static const auto ImmediateType = FSf_Message_Immediate::StaticStruct( );
	if (Type == ImmediateType)
		return false;
	
	static const auto StatefulType = FSf_Message_Stateful::StaticStruct( );
	if (Type == StatefulType)
		return false;

	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UStarfireMessenger::BroadcastImmediateInternal( const FConstStructView &Message, UObject *Context )
{
	const UStruct *MessageType = Message.GetScriptStruct( );
	while (ShouldBroadcastType( MessageType ))
	{
		TArray< const FMessageListener* > Listeners;
		MessageListeners.MultiFind( MessageType, Listeners );

		for (const auto &L : Listeners)
		{
			if ((Context == nullptr) || DoContextsMatch( Context, L->ContextFilter ))
				L->ImmediateCallback( Message, Context );
		}

		MessageType = MessageType->GetSuperStruct( );
	}
}

void UStarfireMessenger::BroadcastStatefulInternal( const FConstStructView &Message, UObject *Context )
{
	const UStruct *MessageType = Message.GetScriptStruct( );
	while (ShouldBroadcastType( MessageType ))
	{
		TArray< const FMessageListener* > Listeners;
		MessageListeners.MultiFind( MessageType, Listeners );

		for (const auto &L : Listeners)
		{
			if ((Context == nullptr) || DoContextsMatch( Context, L->ContextFilter ))
				L->StatefulCallback( Message, Context, EStatefulMessageEvent::NewMessage );
		}

		MessageType = MessageType->GetSuperStruct( );
	}

	FStatefulMessages *MessageData = nullptr;

	if (const auto ExistingData = StatefulMessages.Find( Message.GetScriptStruct( ) ))
	{
		MessageData = *ExistingData;
	}
	else
	{
		MessageData = new FStatefulMessages( );
		StatefulMessages.Add( Message.GetScriptStruct( ), MessageData );
	}

	if (Context != nullptr)
	{
		MessageData->ContextualMessages.Add( Context, FInstancedStruct( Message ) );
	}
	else
	{
		MessageData->DefaultMessage = Message;
	}
}

void UStarfireMessenger::BroadcastStatefulClear( const UStruct *MessageType, const FConstStructView &Message, UObject *Context ) const
{
	while (ShouldBroadcastType( MessageType ))
	{
		TArray< const FMessageListener* > Listeners;
		MessageListeners.MultiFind( MessageType, Listeners );

		if (Context != nullptr)
		{
			for (const auto &L : Listeners)
			{
				if (DoContextsMatch( Context, L->ContextFilter ))
					L->StatefulCallback( Message, Context, EStatefulMessageEvent::Clearing );
			}
		}
		else
		{
			for (const auto &L : Listeners)
				L->StatefulCallback( Message, nullptr, EStatefulMessageEvent::Clearing );
		}

		MessageType = MessageType->GetSuperStruct( );
	}
}

FMessageListenerHandle UStarfireMessenger::StartListeningForMessageInternal( const UScriptStruct *MessageType, TFunction< void( FConstStructView, UObject* )> &&Callback, const UObject *OwningObject, const UObject* ContextFilter )
{
	const auto Listener = CreateListener( MessageType, OwningObject, ContextFilter );

	Listener->ImmediateCallback = MoveTemp( Callback );

	return Listener->Handle;
}

FMessageListenerHandle UStarfireMessenger::StartListeningForStatefulMessageInternal( const UScriptStruct *MessageType, TFunction< void( FConstStructView, UObject*, EStatefulMessageEvent )> &&Callback, const UObject *OwningObject, const UObject* ContextFilter, bool bExpectingContext )
{
	const auto Listener = CreateListener( MessageType, OwningObject, ContextFilter );

	Listener->StatefulCallback = MoveTemp( Callback );

	for (const auto &Entry : StatefulMessages)
	{
		if (!Entry.Key->IsChildOf( MessageType ))
			continue;

		const auto StatefulData = Entry.Value;

		if (bExpectingContext)
		{
			if (ContextFilter != nullptr)
			{
				if (const auto ContextData = StatefulData->ContextualMessages.Find( ContextFilter ))
					Listener->StatefulCallback( *ContextData, const_cast< UObject* >( ContextFilter ), EStatefulMessageEvent::ExistingMessage );
			}
			else
			{
				for (const auto Messages : StatefulData->ContextualMessages)
					Listener->StatefulCallback( Messages.Value, Messages.Key, EStatefulMessageEvent::ExistingMessage );
			}
		}
		else if (StatefulData->DefaultMessage.IsValid( ))
		{
			Listener->StatefulCallback( StatefulData->DefaultMessage, nullptr, EStatefulMessageEvent::ExistingMessage );
		}
	}

	return Listener->Handle;
}

FMessageListener* UStarfireMessenger::CreateListener( const UScriptStruct *MessageType, const UObject *OwningObject, const UObject* ContextFilter )
{
	const auto Listener = new FMessageListener( );

	Listener->Handle.Handle = HandleCounter++;
	Listener->MessageType = MessageType;
	Listener->ContextFilter = ContextFilter;
	Listener->Owner = OwningObject;

	MessageListeners.Add( MessageType, Listener );
	HandleLookups.Add( Listener->Handle, Listener );

	if (OwningObject != nullptr)
		OwnedListeners.Add( OwningObject, Listener );

	return Listener;
}

void UStarfireMessenger::RemoveListener( const FMessageListener *Listener )
{
	MessageListeners.RemoveSingle( Listener->MessageType, Listener );
	HandleLookups.Remove( Listener->Handle );

	if (Listener->Owner)
		OwnedListeners.RemoveSingle( Listener->Owner, Listener );

	delete Listener;
}

void UStarfireMessenger::StopListeningForMessage( FMessageListenerHandle &Handle )
{
	if (!Handle.IsValid( ))
		return;
	
	const auto Listener = HandleLookups.Find( Handle );

	Handle.Invalidate( );

	if (Listener == nullptr)
		return;

	RemoveListener( *Listener );	
}

void UStarfireMessenger::StopListeningForAllMessages( const UObject *OwningObject )
{
	if (OwningObject == nullptr)
		return;
	
	TArray< const FMessageListener* > Listeners;
	OwnedListeners.MultiFind( OwningObject, Listeners );

	for (const auto L : Listeners)
	{
		RemoveListener( L );
	}
}

void UStarfireMessenger::Deinitialize( )
{
	for (const auto &Entry : HandleLookups)
	{
		delete Entry.Value;
	}

	for (const auto &Entry : StatefulMessages)
	{
		delete Entry.Value;
	}

	MessageListeners.Empty( );
	HandleLookups.Empty( );
	OwnedListeners.Empty( );
	StatefulMessages.Empty( );

	Super::Deinitialize( );
}

void UStarfireMessenger::ClearStatefulMessage( bool bExpectingContext, const UScriptStruct *Type, const UObject *Context )
{
	const auto StatefulData = StatefulMessages.Find( Type );
	if (StatefulData == nullptr)
		return;

	if (bExpectingContext)
	{
		if (Context == nullptr)
		{
			for (const auto &Entry : (*StatefulData)->ContextualMessages)
				BroadcastStatefulClear( Type, Entry.Value, Entry.Key );
			
			(*StatefulData)->ContextualMessages.Empty( );
		}
		else
		{
			const auto Contextual = (*StatefulData)->ContextualMessages.Find( Context );
			if (Contextual != nullptr)
			{
				BroadcastStatefulClear( Type, *Contextual, const_cast< UObject* >( Context ) );
				(*StatefulData)->ContextualMessages.Remove( Context );
			}
		}
	}
	else
	{
		BroadcastStatefulClear( Type, (*StatefulData)->DefaultMessage, nullptr );
		
		(*StatefulData)->DefaultMessage.Reset( );
	}
}

bool UStarfireMessenger::HasStatefulMessage( bool bExpectingContext, const UScriptStruct *Type, const UObject *Context ) const
{
	const auto StatefulData = StatefulMessages.Find( Type );
	if (StatefulData == nullptr)
		return false;

	if (bExpectingContext)
	{
		if (Context == nullptr)
			return !(*StatefulData)->ContextualMessages.IsEmpty( );
		
		const auto ContextualData = (*StatefulData)->ContextualMessages.Find( Context );
		if (ContextualData == nullptr)
			return false;

		return ContextualData->IsValid( );
	}

	return (*StatefulData)->DefaultMessage.IsValid( );
}

void UStarfireMessenger::AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector )
{
	CastChecked< UStarfireMessenger >( InThis )->AddReferencedObjects( Collector );
	Super::AddReferencedObjects( InThis, Collector );
}

void UStarfireMessenger::AddReferencedObjects( FReferenceCollector &Collector )
{
	for (auto &Entry : MessageListeners)
	{
		const auto Listener = const_cast< FMessageListener* >( Entry.Value );

		// Based on UDataRegistrySubsystem::AddReferencedObjects this reinterpret is a safe way to pass TObjectPtr to this function
		Collector.MarkWeakObjectReferenceForClearing( reinterpret_cast< UObject** >( &Listener->ContextFilter ), this );
		Collector.MarkWeakObjectReferenceForClearing( reinterpret_cast< UObject** >( &Listener->Owner ), this );

		Collector.AddReferencedObject( Entry.Key );
	}

	for (auto &Entry : OwnedListeners)
	{
		Collector.MarkWeakObjectReferenceForClearing( reinterpret_cast< UObject** >( &Entry.Key ), this );
	}

	for (const auto &Entry : StatefulMessages)
	{
		Entry.Value->DefaultMessage.AddStructReferencedObjects( Collector );

		for (auto &C : Entry.Value->ContextualMessages)
			C.Value.AddStructReferencedObjects( Collector );			
	}
}

void UStarfireMessenger::BroadcastMessage_K2( const int32 &MessageData, UObject *Context )
{
	check( 0 );
}

DEFINE_FUNCTION(UStarfireMessenger::execBroadcastMessage_K2)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn< FStructProperty >( nullptr );

	const auto MessagePtr = Stack.MostRecentPropertyAddress;
	const auto StructProperty = CastField< FStructProperty >( Stack.MostRecentProperty );

	P_GET_OBJECT( UObject, MessageContext );

	P_FINISH;

	if (ensureAlways((StructProperty != nullptr) && (StructProperty->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		const auto bStatefulMessage = FSf_MessageBase::IsMessageTypeStateful( StructProperty->Struct );
		
		if (bStatefulMessage)
			P_THIS->BroadcastStatefulInternal( FConstStructView( StructProperty->Struct, MessagePtr ), MessageContext );
		else
			P_THIS->BroadcastImmediateInternal( FConstStructView( StructProperty->Struct, MessagePtr ), MessageContext );
	}
}

FMessageListenerHandle UStarfireMessenger::StartListeningForMessage_K2( UObject *WorldContext, UScriptStruct *MessageType, UObject *Context, FName FunctionName )
{
	ensureAlways( WorldContext != nullptr );
	ensureAlways( MessageType != nullptr );
	ensureAlways( FunctionName != NAME_None );
	
	TWeakObjectPtr< UObject > WeakObject( WorldContext );
	if (FSf_MessageBase::IsMessageTypeStateful( MessageType ))
	{
		auto Callback = [ WeakObject, FunctionName ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
		{
			if (const auto StrongObject = WeakObject.Get( ))
			{
				const auto Function = StrongObject->FindFunction(FunctionName);
				if (!ensureAlways( Function ))
					return;

				const auto MemoryBlock = FMemory_Alloca_Aligned( Function->ParmsSize, Function->MinAlignment );

				// ChildProperties is a linked list!! We only access it like this because we are 100% sure first property is a FStructProperty that we're looking for.
				const auto MessageDataProperty = CastFieldChecked< FStructProperty >( Function->ChildProperties );
				const auto StructOffsetInFunctionParameters = MessageDataProperty->ContainerPtrToValuePtr< uint8 >( MemoryBlock );

				View.GetScriptStruct( )->InitializeStruct( StructOffsetInFunctionParameters );
				if (MessageDataProperty->Struct == TBaseStructure< FInstancedStruct >::Get( ))
				{
					const FInstancedStruct Temp( View ); // the layout for view and instanced _should_ be identical, but we're not going to bet on it
					
					FMemory::Memcpy( StructOffsetInFunctionParameters, &Temp, sizeof( Temp ) );
				}
				else
				{
					View.GetScriptStruct( )->CopyScriptStruct( StructOffsetInFunctionParameters, View.GetMemory( ) );
				}

				// Enumeration parameter is required and is next
				const auto EnumProperty = CastFieldChecked< FEnumProperty >( MessageDataProperty->Next );
				EnumProperty->SetSingleValue_InContainer( MemoryBlock, &Type, 0 );

				if (Function->NumParms > 2) // Context is an optional parameter
				{
					const auto ContextDataProperty = CastFieldChecked< FObjectProperty >( EnumProperty->Next );
					const auto ContextOffsetInFunctionParameters = ContextDataProperty->ContainerPtrToValuePtr< uint8 >( MemoryBlock );
					ContextDataProperty->SetObjectPropertyValue( ContextOffsetInFunctionParameters, Context );
				}

				StrongObject->ProcessEvent( Function, MemoryBlock );
			}
		};

		const auto bExpectingContext = FSf_MessageBase::GetContextType( MessageType ) != nullptr;
		
		return StartListeningForStatefulMessageInternal( MessageType, Callback, WorldContext, Context, bExpectingContext );
	}
	else
	{
		auto Callback = [ WeakObject, FunctionName ]( const FConstStructView &View, UObject *Context ) -> void
		{
			if (const auto StrongObject = WeakObject.Get( ))
			{
				const auto Function = StrongObject->FindFunction( FunctionName );
				if (!ensureAlways( Function ))
					return;

				const auto MemoryBlock = FMemory_Alloca_Aligned( Function->ParmsSize, Function->MinAlignment );

				// ChildProperties is a linked list!! We only access it like this because we are 100% sure first property is a FStructProperty that we're looking for.
				const auto MessageDataProperty = CastFieldChecked< FStructProperty >( Function->ChildProperties );
				const auto StructOffsetInFunctionParameters = MessageDataProperty->ContainerPtrToValuePtr< uint8 >( MemoryBlock );

				View.GetScriptStruct( )->InitializeStruct( StructOffsetInFunctionParameters );
				if (MessageDataProperty->Struct == TBaseStructure< FInstancedStruct >::Get( ))
				{
					const FInstancedStruct Temp( View ); // the layout for view and instanced _should_ be identical, but we're not going to bet on it
					
					FMemory::Memcpy( StructOffsetInFunctionParameters, &Temp, sizeof( Temp ) );
				}
				else
				{
					View.GetScriptStruct( )->CopyScriptStruct( StructOffsetInFunctionParameters, View.GetMemory( ) );
				}

				if (Function->NumParms > 1) // Context is an optional parameter
				{
					const auto ContextDataProperty = CastFieldChecked< FObjectProperty >( MessageDataProperty->Next );
					const auto ContextOffsetInFunctionParameters = ContextDataProperty->ContainerPtrToValuePtr< uint8 >( MemoryBlock );
					ContextDataProperty->SetObjectPropertyValue( ContextOffsetInFunctionParameters, Context );
				}

				StrongObject->ProcessEvent( Function, MemoryBlock );
			}
		};

		return StartListeningForMessageInternal( MessageType, Callback, WorldContext, Context );
	}
}

void UStarfireMessenger::StopListeningForMessage_BP( FMessageListenerHandle &Handle )
{
	StopListeningForMessage( Handle );
}

void UStarfireMessenger::StopListeningForAllMessages_BP( const UObject *WorldContext )
{
	StopListeningForAllMessages( WorldContext );
}

void UStarfireMessenger::ClearStatefulMessage_K2( const UScriptStruct *MessageType, const UObject *Context, bool bExpectingContext )
{
	ClearStatefulMessage( bExpectingContext, MessageType, Context );
}

bool UStarfireMessenger::HasStatefulMessage_K2( const UScriptStruct *MessageType, const UObject *Context, bool bExpectingContext ) const
{
	return HasStatefulMessage( bExpectingContext, MessageType, Context );
}
