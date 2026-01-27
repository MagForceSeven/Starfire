
#pragma once

#include "UObject/ObjectMacros.h"
#include "Templates/TypeUtilitiesSF.h"

#include "MessageTypes.generated.h"

/*
 *  - Statefulness: The system allows messages to "fire and forget", but also allows for them to maintain state and be cleared when needed
 *					This allows the system to provide an easy way to handle "join in progress" situations where a listener needs to know that an message has already occurred and to get
 *					access to the data & context that it was comprised of.
 *				- Only make messages "stateful" if you need them to be. Don't use the system as a crutch to solve all your edge case needs. Being "stateful" costs memory, and even
 *				  though everything is fairly efficient, it can eventually add up.
 *
 *  - Hierarchical: When listening for messages types, listeners will also receive messages of child types (only supported for C++ message types)
 *				In both C++ and Blueprint, there are alternate params that allow for determining the actual message type safely if that is needed
 *				- A macro (`SET_MESSAGE_TYPE_AS_ABSTRACT`) is provided to configure messages types that are meant to act as a parent type, can be listened for but should not be broadcast directly.
 *				- Parent types do not _have_ to be abstract, but the support is provided for the cases where it is desirable.
 *
 *	- Message Types come in two flavors based that statefulness: Stateful and Immediate
 *	- Message Types can specify a ContextType that
 *			a) defines whether or not the message requires a context to broadcast
 *			b) when a context is required, defines the type of object that will be used as a context when the message is broadcast
 *			c) when a context is required, provide a strong guarantee that the context is actually valid (ie listeners shouldn't have to null check the context under most circumstances)
 *
 *			- to specify a context type, in the Message structure you just add a line that looks like: "SET_CONTEXT_TYPE(XXXXXX)"
 *				- XXXXXX could be any UObject derived type
 *				- The "ContextType" is inherited through parent types, which allows you to declare this ContextType in a common base structure and not require repeats in each child type
 *				- The "ContextType" can also be re-declared in child types and it will update the type
 *					- When doing this, the best practice is that these re-declarations should be more narrow than the parent
 *						- If EventA has ContextType = APawn, it's reasonable for EventB (derived from EventA) to redeclare ContextType = ACharacter (since ACharacter derives from APawn)
 *							However EventB (probably) shouldn't redeclare ContextType = AAIController (since it's unrelated to APawn as a child)
 *				- The type being specified as the "ContextType" does need a full definition available, only having access to a forward declaration isn't sufficient
 *					- This shouldn't really be a huge issue since context related events are probably declared with the context or with other events that share a context type
 *						and where the listener for that message likely would have to/want to include that header anyway
 *						
 *			- the SET_CONTEXT_NAME macro can be used to set the name the context pin is given in blueprints
 *				- SET_CONTEXT_NAME( "PinName" )
 *				- if unset, will default to "Context"
 *				- Will also be used to set the name of the context pin when creating matching functions or custom events
 */

// Template that can be specialized to identify message types
template < class TType >
constexpr bool Sf_MessageType_IsAbstract( ) { return false; }

// Helper macro to make it easier for clients to declare the specialization correctly
#if !WITH_EDITORONLY_DATA
#define SET_MESSAGE_TYPE_AS_ABSTRACT( Type ) \
	template < >	\
	constexpr bool Sf_MessageType_IsAbstract< Type >( ) { return true; }
#else
#define SET_MESSAGE_TYPE_AS_ABSTRACT( Type ) \
	template < >	\
	constexpr bool Sf_MessageType_IsAbstract< Type >( ) { return true; } \
	static const FAbstractMarker Type##Marker( #Type );

// A temporary type that allows us to keep track of abstract types in editor builds for editor reasons
struct STARFIREMESSENGER_API FAbstractMarker
{
	explicit FAbstractMarker( const char* Typename );
};
#endif

// Base type for all messages supported by Starfire Messenger
// This is purposefully not fully exported. Client messages should derive from FSf_Message_Immediate or FSf_Message_Stateful. Not directly from this structure
USTRUCT( )
struct FSf_MessageBase
{
	GENERATED_BODY( )
public:
	// The expected context type for messages. Hidden/Overridden by derived types using the SET_CONTEXT_TYPE macro
	using ContextType = nullptr_t;

	// Utility for checking if a message type is stateful at runtime
	[[nodiscard]] STARFIREMESSENGER_API static bool IsMessageTypeStateful( const UScriptStruct *MessageType );
	// Utility for the runtime check necessary for Stateful messages having associated context data
	[[nodiscard]] STARFIREMESSENGER_API static bool DoesStatefulTypeRequireContext( const UScriptStruct *MessageType );
#if WITH_EDITORONLY_DATA
	// Utility for finding a message types context type at runtime
	[[nodiscard]] STARFIREMESSENGER_API static TSoftClassPtr< UObject > GetContextType( const UScriptStruct *MessageType );
	// Utility for runtime checking if a type is abstract
	[[nodiscard]] STARFIREMESSENGER_API static bool IsMessageTypeAbstract( const UScriptStruct *MessageType );
	// Utility for getting the override name to give context pins of the specified type
	[[nodiscard]] STARFIREMESSENGER_API static FText GetContextPinName( const UScriptStruct *MessageType );

private:
	friend class FStarfireMessengerEditor;
	// Use the pre-main-init data to construct the desired Editor runtime data
	STARFIREMESSENGER_API static void RemapMessageContextEditorData( );
#endif
	friend class FStarfireMessenger;
	STARFIREMESSENGER_API static void RemapMessageContextData( );
};
SET_MESSAGE_TYPE_AS_ABSTRACT( FSf_MessageBase )

// Macro to update the context type for derived types
#define SET_CONTEXT_TYPE( Type ) \
	using ContextType = Type; \
	static inline const FMessageContextTypeMarker ContextTypeMarker{ &StaticStruct, &std::remove_cv_t< Type >::StaticClass };

// An intermediary type that allows us to keep track of context type for Editor tooling and runtime functionality
struct STARFIREMESSENGER_API FMessageContextTypeMarker
{
	FMessageContextTypeMarker( UScriptStruct* (*StructGetter)(void), UClass*(*TypeGetter)(void) );
};

#if !WITH_EDITORONLY_DATA
	#define SET_CONTEXT_NAME( ... )
#else
	#define SET_CONTEXT_NAME( ContextPinName )	\
	static inline const FMessageContextNameMarker ContextNameMarker{ &StaticStruct, ContextPinName };

// An intermediary type that allows us to keep track of abstract types in editor builds for editor reasons
struct STARFIREMESSENGER_API FMessageContextNameMarker
{
	FMessageContextNameMarker( UScriptStruct* (*StructGetter)(void), const char* ContextName );
};
#endif

// Base for standard fire-and-forget messages
USTRUCT( BlueprintType )
struct STARFIREMESSENGER_API FSf_Message_Immediate : public FSf_MessageBase
{
	GENERATED_BODY( )
public:
};
SET_MESSAGE_TYPE_AS_ABSTRACT( FSf_Message_Immediate )

// Base for messages that are stateful and should be stored for delivery to later listeners
USTRUCT( BlueprintType )
struct STARFIREMESSENGER_API FSf_Message_Stateful : public FSf_MessageBase
{
	GENERATED_BODY( )
public:
};
SET_MESSAGE_TYPE_AS_ABSTRACT( FSf_Message_Stateful )

// A collection of contexts that can be used for compile checks against message types
template < class TType >
concept CLibraryMessageType = SFstd::same_as< TType, FSf_MessageBase > || SFstd::same_as< TType, FSf_Message_Immediate > || SFstd::same_as< TType, FSf_Message_Stateful >;

template < class TType >
concept CMessengerMessageType = SFstd::derived_from< TType, FSf_MessageBase > && !CLibraryMessageType< TType >;

template < class TType >
concept CImmediateMessageType = SFstd::derived_from< TType, FSf_Message_Immediate > && !CLibraryMessageType< TType >;

template < class TType >
concept CStatefulMessageType = SFstd::derived_from< TType, FSf_Message_Stateful > && !CLibraryMessageType< TType >;

template < class TType >
concept CMessageNoContext = std::is_null_pointer_v< typename TType::ContextType >;

template < class TType >
concept CMessageWithContext = !std::is_null_pointer_v< typename TType::ContextType >;

template < class TType >
concept CImmediateNoContextType = CImmediateMessageType< TType > && CMessageNoContext< TType >;

template < class TType >
concept CImmediateWithContextType = CImmediateMessageType< TType > && CMessageWithContext< TType >;

template < class TType >
concept CStatefulNoContextType = CStatefulMessageType< TType > && CMessageNoContext< TType >;

template < class TType >
concept CStatefulWithContextType = CStatefulMessageType< TType > && CMessageWithContext< TType >;

template < class TType >
concept CAbstractMessageType = Sf_MessageType_IsAbstract< TType >( );