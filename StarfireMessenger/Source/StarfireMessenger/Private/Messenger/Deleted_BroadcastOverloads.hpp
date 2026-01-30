
// ReSharper disable once CppMissingIncludeGuard
#ifndef STARFIRE_MESSENGER_HPP
	#error You shouldn't be including this file directly
#endif

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
private:
