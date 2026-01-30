
#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "Messenger/MessengerTypes.h"
#include "Messenger/MessageTypes.h"

#include "StructUtils/StructView.h"

#include "Messenger.generated.h"

/*
 *	- Listener Callbacks:
 *	    - The format of these callbacks is strictly enforced by the compiler
 *	    - The format is well defined based on the properties of the Message being listened to (the Statefulness and the ContextType)
 *
 *		- Parameter 1: const MessageType& or const TConstStructView< MessageType >& or TInstancedStruct< MessageType >&
 *			- This is a parameter to all listener callbacks
 *			- A const reference to the message being broadcast
 *			- This is the same type that is used for the template parameter when calling StartListeningForMessage
 *				- If the template parameter is omitted, this is the parameter that will end up being used to deduce that template parameter
 *			- The TConstStructView/TInstancedStruct type comes into play when wanting to take full advantage of the hierarchical listening, but it is not required
 *				- Struct View is used for C++ and the Instanced Struct is used for Blueprint
 *
 *		- Stateful Parameter 2: EStatefulMessageEvent
 *			- This parameter is only part of the callback when the MessageType derives from FSf_Message_Stateful
 *			- This is a enumeration that indicates the reason for the stateful call
 *				- An original broadcast (same as immediate messages)
 *				- A new listener (this is the statefulness of the message)
 *				- The message is being cleared and the data about to be cleared
 *
 *		- Parameter 2 (or Stateful Parameter 3) (Optional): ContextType*
 *			- This is a parameter to listener callbacks if the MessageType (or one of it's parents) have declared a ContextType
 *			- This parameter is the ContextObject that was specified by the caller to Broadcast
 *			- The subsystem provides a strong guarantee that this pointer is valid prior to being dispatched to listeners
 *				- All messages that specify a ContextType will not be broadcast (and will ensure) if a caller attempts to broadcast with a nullptr
 *				- This means that Immediate message and the ::NewMessage case of Stateful messages can rely on ContextObject being valid
 *				- The other two cases for Stateful messages are trickery, but the guarantee is still applicable
 *					- For the ::ExistingMessage case, if the context has been lost the broadcast will not occur (no ensure)
 *					- For the ::Clearing case, if the message is being cleared for a specific context this parameter will be valid
 *					- For the ::Clearing case, if the message is being cleared for *all* contexts, this parameter will either not be passed (message type requires no context)
 *						- or the message type does require a context and it is valid and not nullptr or has been lost and the clear will not be broadcast (no ensure)
 *
 *		Example Messages:
 *		
 *		struct ExampleA : public FSf_Message_Immediate
 *
 *		struct ExampleB : public FSf_Message_Immediate
 *			SET_CONTEXT_TYPE( APawn )
 *
 *		struct ExampleC1 : public ExampleB
 *
 *		struct ExampleC2 : public ExampleB
 *			SET_CONTEXT_TYPE( ACharacter )
 *
 *		struct Example1 : public FSf_Message_Stateful
 *
 *		struct Example2 : public FSf_Message_Stateful
 *			SET_CONTEXT_TYPE( APawn )
 *
 *		struct Example31 : public Example2
 *
 *		struct Example32 : public Example2
 *			SET_CONTEXT_TYPE( ACharacter )
 *
 *		ExampleListeners:
 *
 *		void ListenerA( const ExampleA& )
 *			- Simple immediate listener, no context data
 *
 *		void ListenerB( const ExampleB&, APawn* )
 *			- Immediate listener, context data of type APawn
 *			- Could be used to listen for any child of ExampleB, but no child data would be accessible
 *			
 *		void ListenerC1( const ExampleC1&, APawn* )
 *			- Immediate listener, context data of type APawn based on parent type
 *
 *		void ListenerC2( const ExampleC2&, ACharacter* )
 *			- Immediate listener, context data changed from parent to ACharacter
 *
 *		void ListenerB_Alt( const TConstStructView< ExampleB >&, APawn* )
 *			- Immediate listener, context data of type APawn
 *			- Could be used to listen for any child of ExampleB, and cast for child type data
 *
 *		void Listener1( const Example1&, EStatefulMessageEvent )
 *			- Stateful listener, with type for statefulness of broadcast
 *
 *		void Listener2( const Example2&, APawn*, EStatefulMessageEvent )
 *			- Stateful listener, context data of type APawn, and type for statefulness of broadcast
 *			- Could be used to listen for any child of Example2, but no child data would be accessible
 *
 *		void Listener32( const Example32&, ACharacter*, EStatefulMessageEvent )
 *			- Stateful listener, context data changed from parent to ACharacter
 *
 *		void Listener2_Alt( const TConstStructView< Example2 >&, APawn*, EStatefulMessageEvent )
 *			- Stateful listener, context data of type APawn, and type for statefulness of broadcast
 *			- Could be used to listen for any child of Example2 (Example31 or Example32), and cast for child type data
 *			
 */

struct FMessageListener;
struct FStatefulMessages;

// A concept to check for a constructor with a specific set of parameters
template < class type_t, class ... args_t >
concept CConstructorVarArgsMatch = requires( args_t && ... args )
{
	type_t( std::forward< args_t >( args ) ... );
};

// A concept used for the deleted contextual Broadcast options, to check if a constructor is supported as long as the last type is not the same as the context type
template < class type_t, class ... args_t >
concept CContextlessConstructorVarArgsMatch = requires( args_t && ... args )
{
	type_t( std::forward< args_t >( args ) ... );
	!std::is_same_v< std::tuple_element_t< std::tuple_size_v< args_t ... > - 1, std::tuple< args_t ... > >, typename type_t::ContextType* >;
};

// Class that acts as a centralized message bus to pass messages from indeterminate contexts, to some collection of unknown listeners
UCLASS( )
class STARFIREMESSENGER_API UStarfireMessenger : public UWorldSubsystem
{
	GENERATED_BODY( )
public:
	// Subsystem Accessor
	[[nodiscard]] static UStarfireMessenger* GetSubsystem( const UObject *WorldContext );

	// Broadcast an immediate message out to the listeners that have registered for it
	template < CImmediateNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void Broadcast( const type_t &Message );
	template < CImmediateWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void Broadcast( const type_t &Message, typename type_t::ContextType *Context );
	// Broadcast a stateful message out to the listeners that have registered for it
	// And store message for later broadcast to new listeners
	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void Broadcast( const type_t &Message );
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void Broadcast( const type_t &Message, typename type_t::ContextType *Context );

	// Overloads that allow for in-place construction of the event data
	
	// Broadcast an immediate message out to the listeners that have registered for it
	template < CImmediateNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	void Broadcast( args_t && ... args );
	template < CImmediateWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	void Broadcast( typename type_t::ContextType *Context, args_t && ... args );
	// Broadcast a stateful message out to the listeners that have registered for it
	// And store message for later broadcast to new listeners
	template < CStatefulNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	void Broadcast( args_t && ... args );
	template < CStatefulWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	void Broadcast( typename type_t::ContextType *Context, args_t && ... args );
	
	// Clear the stateful message for the specified message type
	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void ClearStatefulMessage( );
	// Clear any stateful message for the specified message type and context
	// A nullptr context will clear stateful messages for all contexts
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	void ClearStatefulMessage( typename type_t::ContextType *Context );

	// curious, for some reason the [[nodiscard]] attribute causes a compile error in MSVC, so we'll drop it for now in favor of the "requires" clause which is more important

	// Check if a particular message type has an active stored message
	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	/*[[nodiscard]]*/ bool HasStatefulMessage( ) const;
	// Check if a particular message type has an active stored message for a potential context
	// A nullptr context will check for stored messages for any context
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	/*[[nodiscard]]*/ bool HasStatefulMessage( typename type_t::ContextType *Context ) const;

	// Stop listening for the message
	void StopListeningForMessage( FMessageListenerHandle &Handle );

	// Unregister an object from all the messages that it has requested to listen for
	void StopListeningForAllMessages( const UObject *OwningObject );

	// Overloads to start listening for a message that does not require a context
	template < CImmediateNoContextType type_t >
	FMessageListenerHandle StartListeningForMessage( TFunction< void ( const type_t& )> &&Callback );
	template < CImmediateNoContextType type_t >
	FMessageListenerHandle StartListeningForMessage( const UObject *Owner, TFunction< void ( const type_t& )> &&Callback );

	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t& ) );
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t& ) const );

	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) );
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) const );

	// Overloads to start listening for a message that requires a context
	template < CImmediateWithContextType type_t >
	FMessageListenerHandle StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter = nullptr );
	template < CImmediateWithContextType type_t >
	FMessageListenerHandle StartListeningForMessage( const UObject *Owner, TFunction< void ( const type_t&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter = nullptr );
	
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr );

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr );
	
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr );

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr );

	// Overloads to start listening for a stateful message that does not require a context
	template < CStatefulNoContextType type_t >
	FMessageListenerHandle StartListeningForMessage( TFunction< void ( const type_t&, EStatefulMessageEvent )> &&Callback );
	template < CStatefulNoContextType type_t >
	FMessageListenerHandle StartListeningForMessage( const UObject *Owner, TFunction< void ( const type_t&, EStatefulMessageEvent )> &&Callback );

	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, EStatefulMessageEvent ) );
	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, EStatefulMessageEvent ) const );

	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) );
	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) const );

	// Overloads to start listening for a stateful message that requires a context
	template < CStatefulWithContextType type_t >
	FMessageListenerHandle StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter = nullptr );
	template < CStatefulWithContextType type_t >
	FMessageListenerHandle StartListeningForMessage( const UObject *Owner, TFunction< void ( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter = nullptr );
	
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr );

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr );

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr );

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr );
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr );

	// Subsystem API
	void Deinitialize( ) override;
	bool ShouldCreateSubsystem( UObject *Outer ) const override;

	// GC hooks to prevent message data from being lost
	static void AddReferencedObjects( UObject *InThis, FReferenceCollector &Collector );
	void AddReferencedObjects( FReferenceCollector &Collector );

private:
	// Non-template utilities for broadcasting a new message to potential listeners
	void BroadcastImmediateInternal( const FConstStructView &Message, UObject *Context );
	void BroadcastStatefulInternal( const FConstStructView &Message, UObject *Context );

	// Broadcast to listeners that stateful message data is being cleared
	void BroadcastStatefulClear( const UStruct *MessageType, const FConstStructView &Message, UObject *Context ) const;

	// Non-template utilities for establishing a new message listener
	[[nodiscard]] FMessageListenerHandle StartListeningForMessageInternal( const UScriptStruct *MessageType, TFunction< void( FConstStructView, UObject* )> &&Callback, const UObject *OwningObject, const UObject* ContextFilter );
	[[nodiscard]] FMessageListenerHandle StartListeningForStatefulMessageInternal( const UScriptStruct *MessageType, TFunction< void( FConstStructView, UObject*, EStatefulMessageEvent )> &&Callback, const UObject *OwningObject, const UObject* ContextFilter, bool bExpectingContext );

	// Allocate and configure a new listener
	[[nodiscard]] FMessageListener* CreateListener( const UScriptStruct *MessageType, const UObject *OwningObject, const UObject* ContextFilter );
	// Cleanup and deallocate a listener
	void RemoveListener( const FMessageListener *Listener );

	// Non-template utility to clear the stateful message data for a message type & context
	void ClearStatefulMessage( bool bExpectingContext, const UScriptStruct *Type, const UObject *Context );

	// Non-template utility for checking the existence of stateful message data for a message type & context
	[[nodiscard]] bool HasStatefulMessage( bool bExpectingContext, const UScriptStruct *Type, const UObject *Context ) const;

	// All listeners that are interested in a particular type of message occurring
	TMultiMap< TObjectPtr< const UStruct >, const FMessageListener* > MessageListeners;

	// All message types that a specific listener is interested in
	TMultiMap< TObjectPtr< const UObject >, const FMessageListener* > OwnedListeners;

	// Direct association of the unique ids to the corresponding listener instance
	TMap< FMessageListenerHandle, const FMessageListener* > HandleLookups;

	// The collection of stateful messages that have been sent in the past and should be dispatched to any new listeners
	TMap< TObjectPtr< const UScriptStruct >, FStatefulMessages* > StatefulMessages;

	// Incremental counter for creating a unique identifier for each listener
	int HandleCounter = 1;

	// Check whether this handle refers to an event registration
	UFUNCTION(BlueprintPure, Category = "Starfire Messenger|Handle", meta = (DisplayName = "Is Valid"))
	static bool Handle_IsValid_BP( const FMessageListenerHandle &Handle ) { return Handle.IsValid(); }

	friend class UK2Node_BroadcastEvent;
	friend class UK2Node_RegisterForEvent;

	// Broadcast a message out to any registered listeners
	UFUNCTION( BlueprintCallable, BlueprintInternalUseOnly, CustomThunk, meta = (CustomStructureParam = "MessageData") )
	void BroadcastMessage_K2( const int32 &MessageData, UObject *Context );
	DECLARE_FUNCTION(execBroadcastMessage_K2);

	// Register for messages of a specific type and possibly from a specific context
	UFUNCTION( BlueprintCallable, BlueprintInternalUseOnly, meta = (WorldContext = "WorldContext") )
	FMessageListenerHandle StartListeningForMessage_K2( UObject *WorldContext, UScriptStruct *MessageType, UObject *Context, FName FunctionName );

	// Unregister for a specific message registered for previously
	UFUNCTION( BlueprintCallable, Category = "Starfire Messenger", DisplayName = "Stop Listening for Message" )
	void StopListeningForMessage_BP( UPARAM( ref ) FMessageListenerHandle &Handle );

	// Unregister from all the messages this object is registered for
	UFUNCTION( BlueprintCallable, Category = "Starfire Messenger", DisplayName = "Stop Listening for All Messages", meta = (WorldContext = "WorldContext") )
	void StopListeningForAllMessages_BP( const UObject *WorldContext );

	// Remove any existing stateful message data for a message type and possibly for a specific context
	UFUNCTION( BlueprintCallable, BlueprintInternalUseOnly )
	void ClearStatefulMessage_K2( const UScriptStruct *MessageType, const UObject *Context, bool bExpectingContext );

	// Check if there is any existing stateful message data for a type and possibly for a specific context
	UFUNCTION( BlueprintCallable, BlueprintInternalUseOnly )
	bool HasStatefulMessage_K2( const UScriptStruct *MessageType, const UObject *Context, bool bExpectingContext ) const;

	friend class UK2Node_BroadcastMessage;
	friend class UK2Node_RegisterForMessage;
	friend class UK2Node_StartListeningForMessage;
	friend class UK2Node_Messenger_ClearStateful;
	friend class UK2Node_Messenger_HasStateful;

	// Deleted & deprecated versions of the Broadcast functions to produce better error results than a bunch of non-matching overloads
	// Deleted to produce the most helpful error
	// Deprecated to also produce a more specific user error for why it is disallowed
public:
	template < CImmediateNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const type_t &Message ) = delete;
	template < CImmediateWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const type_t &Message, typename type_t::ContextType *Context ) = delete;
	template < CStatefulNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const type_t &Message ) = delete;
	template < CStatefulWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const type_t &Message, typename type_t::ContextType *Context ) = delete;

	template < CImmediateNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( args_t && ... args ) = delete;
	template < CImmediateWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( typename type_t::ContextType *Context, args_t && ... args ) = delete;
	template < CStatefulNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( args_t && ... args ) = delete;
	template < CStatefulWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( typename type_t::ContextType *Context, args_t && ... args ) = delete;

	template < CImmediateNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const UObject *Context, args_t && ... args ) = delete;
	template < CImmediateWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( args_t && ... args ) = delete;
	template < CStatefulNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( const UObject *Context, args_t && ... args ) = delete;
	template < CStatefulWithContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast an abstract message type.")]]
	void Broadcast( args_t && ... args ) = delete;

	template < CImmediateNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message with no context type with a context input")]]
	void Broadcast( const type_t &Message, const UObject* Context ) = delete;
	template < CImmediateWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( const type_t &Message ) = delete;
	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message with no context type with a context input")]]
	void Broadcast( const type_t &Message, const UObject* ) = delete;
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( const type_t &Message ) = delete;

	template < CImmediateNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message with no context type with a context input")]]
	void Broadcast( const UObject *Context, args_t && ... args ) = delete;
	template < CImmediateWithContextType type_t, class ... args_t >
		requires (CContextlessConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( args_t && ... args ) = delete;
	template < CStatefulNoContextType type_t, class ... args_t >
		requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message with no context type with a context input")]]
	void Broadcast( const UObject *Context, args_t && ... args ) = delete;
	template < CStatefulWithContextType type_t, class ... args_t >
		requires (CContextlessConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( args_t && ... args ) = delete;
	
	template < CImmediateWithContextType type_t >
		requires (CConstructorVarArgsMatch< type_t > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( ) = delete;
	template < CStatefulWithContextType type_t >
		requires (CConstructorVarArgsMatch< type_t > && !CAbstractMessageType< type_t >)
	[[deprecated("Attempting to broadcast a message that requires a context without a context input")]]
	void Broadcast( ) = delete;
	
	template < CStatefulNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	void ClearStatefulMessage( ) = delete;
	template < CStatefulNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	void ClearStatefulMessage( const UObject *Context ) = delete;
	template < CStatefulWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	void ClearStatefulMessage( typename type_t::ContextType *Context ) = delete;
	template < CStatefulWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	void ClearStatefulMessage( ) = delete;

	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for a non-context message type, using a context.")]]
	void ClearStatefulMessage( const UObject *Context ) = delete;
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for message type requiring a context, without a context. Pass nullptr to clear all contextual context data.")]]
	void ClearStatefulMessage( ) = delete;

	template < CStatefulNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( ) const = delete;
	template < CStatefulNoContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( const UObject *Context ) const = delete;
	template < CStatefulWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( typename type_t::ContextType *Context ) const = delete;
	template < CStatefulWithContextType type_t >
		requires (CAbstractMessageType< type_t >)
	[[deprecated("Attempting to clear stateful data for an abstract message type.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( ) const = delete;
	
	template < CStatefulNoContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to check stateful data for a non-context message type, using a context.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( const UObject *Context ) const = delete;
	template < CStatefulWithContextType type_t >
		requires (!CAbstractMessageType< type_t >)
	[[deprecated("Attempting to check stateful data for message type requiring a context, without a context. Pass nullptr to check for any contextual context data.")]]
	/*[[nodiscard]]*/ bool HasStatefulMessage( ) const = delete;
	
	// Deleted & deprecated versions of the Listening functions to produce better error results than a bunch of non-matching overloads
	// Deleted to produce the most helpful error
	// Deprecated to also produce a more specific user error for why it is disallowed
public:
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t& ) ) = delete;
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t& ) const ) = delete;

	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) ) = delete;
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) const ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	
	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, EStatefulMessageEvent ) ) = delete;
	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, EStatefulMessageEvent ) const ) = delete;

	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) ) = delete;
	template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) const ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, const typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, const typename other_type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>* ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>* ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const other_type_t&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< type_t, other_type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* >)
	[[deprecated("Attempting to bind handler with non-const context when message type requires a const context.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;

	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter = nullptr ) = delete;
	template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t = UObject >
		requires (SFstd::derived_from< other_type_t, type_t > && !SFstd::is_mutable_pointer< typename other_type_t::ContextType* > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, std::remove_cv_t<typename other_type_t::ContextType>*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter = nullptr ) = delete;
};

#if CPP
#define STARFIRE_MESSENGER_HPP
#include "../../Private/Messenger/Messenger.hpp"
#undef STARFIRE_MESSENGER_HPP
#endif