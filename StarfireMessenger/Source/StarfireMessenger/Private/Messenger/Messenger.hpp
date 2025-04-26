
#ifndef STARFIRE_MESSENGER_HPP
	#error You shouldn't be including this file directly
#endif

#include "StructView.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CImmediateNoContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( const type_t &Message )
{
	BroadcastImmediateInternal( FConstStructView::Make< type_t >( Message ), nullptr );
}

template < CImmediateWithContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( const type_t &Message, typename type_t::ContextType *Context )
{
	if (ensureAlways( Context != nullptr ))
	{
		auto MutableContext = const_cast< std::remove_cv_t< typename type_t::ContextType > * >( Context );
		BroadcastImmediateInternal( FConstStructView::Make< type_t >( Message ), MutableContext );
	}
}

template < CStatefulNoContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( const type_t &Message )
{
	BroadcastStatefulInternal( FConstStructView::Make< type_t >( Message ), nullptr );
}

template < CStatefulWithContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( const type_t &Message, typename type_t::ContextType *Context )
{
	if (ensureAlways( Context != nullptr ))
	{
		auto MutableContext = const_cast< std::remove_cv_t< typename type_t::ContextType > * >( Context );
		BroadcastStatefulInternal( FConstStructView::Make< type_t >( Message ), MutableContext );
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CImmediateNoContextType type_t, class ... args_t >
	requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( args_t && ... args )
{
	const type_t Message( std::forward< args_t >( args ) ... );
	BroadcastImmediateInternal( FConstStructView::Make< type_t >( Message ), nullptr );
}

template < CImmediateWithContextType type_t, class ... args_t >
	requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( typename type_t::ContextType *Context, args_t && ... args )
{
	if (ensureAlways( Context != nullptr ))
	{
		const type_t Message( std::forward< args_t >( args ) ... );
		auto MutableContext = const_cast< std::remove_cv_t< typename type_t::ContextType > * >( Context );
		BroadcastImmediateInternal( FConstStructView::Make< type_t >( Message ), MutableContext );
	}
}

template < CStatefulNoContextType type_t, class ... args_t >
	requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( args_t && ... args )
{
	const type_t Message( std::forward< args_t >( args ) ... );
	BroadcastStatefulInternal( FConstStructView::Make< type_t >( Message ), nullptr );
}

template < CStatefulWithContextType type_t, class ... args_t >
	requires (CConstructorVarArgsMatch< type_t, args_t ... > && !CAbstractMessageType< type_t >)
void UStarfireMessenger::Broadcast( typename type_t::ContextType *Context, args_t && ... args )
{
	if (ensureAlways( Context != nullptr ))
	{
		const type_t Message( std::forward< args_t >( args ) ... );
		auto MutableContext = const_cast< std::remove_cv_t< typename type_t::ContextType > * >( Context );
		BroadcastStatefulInternal( FConstStructView::Make< type_t >( Message ), MutableContext );
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CImmediateNoContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t& )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		const auto &Message = View.Get< const type_t >( );

		InnerCallback( Message );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, nullptr );
}

template < CImmediateNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t& ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );

			(StrongOwner->*Callback)( Message );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

template < CImmediateNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t& ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );

			(StrongOwner->*Callback)( Message );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

template < CImmediateNoContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >& )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		InnerCallback( TypedView );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, nullptr );
}

template < CImmediateNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >& ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

template < CImmediateNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >& ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t >
	requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >& )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		InnerCallback( TypedView );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, nullptr );
}

template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t >
	requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t, class owner_t >
	requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >& ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, nullptr );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CImmediateWithContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		const auto &Message = View.Get< const type_t >( );
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( Message, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CImmediateWithContextType type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		const auto &Message = View.Get< const type_t >( );
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( Message, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >&, typename type_t::ContextType* )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext );
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType* ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

template < CImmediateWithContextType type_t, CImmediateWithContextType other_type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType* ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext );
		}
	};
	return StartListeningForMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CStatefulNoContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t&, EStatefulMessageEvent )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		const auto &Message = View.Get< const type_t >( );

		InnerCallback( Message, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, nullptr, false );
}

template < CStatefulNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t&, EStatefulMessageEvent ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );

			(StrongOwner->*Callback)( Message, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

template < CStatefulNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t&, EStatefulMessageEvent ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );

			(StrongOwner->*Callback)( Message, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

template < CStatefulNoContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >&, EStatefulMessageEvent )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		InnerCallback( TypedView, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, nullptr, false );
}

template < CStatefulNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, EStatefulMessageEvent ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

template < CStatefulNoContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, EStatefulMessageEvent ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t >
		requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >&, EStatefulMessageEvent )> &&Callback )
{
	const auto MessageType = type_t::StaticStruct( );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		InnerCallback( TypedView, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, nullptr, false );
}

template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t >
		requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

template < CStatefulNoContextType type_t, CStatefulNoContextType other_type_t, class owner_t >
		requires SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, EStatefulMessageEvent ) const )
{
	const auto MessageType = type_t::StaticStruct( );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			(StrongOwner->*Callback)( TypedView, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, nullptr, false );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CStatefulWithContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		const auto &Message = View.Get< const type_t >( );
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( Message, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > &&  SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t >
	requires (!SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >)
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CStatefulWithContextType type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		const auto &Message = View.Get< const type_t >( );
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( Message, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const type_t&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			const auto &Message = View.Get< const type_t >( );
			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( Message, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );
		
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( TFunction< void ( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent )> &&Callback, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );

	auto BoxedCallback = [ InnerCallback = MoveTemp( Callback ) ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		TConstStructView< other_type_t > TypedView;
		TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );
		
		const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

		InnerCallback( TypedView, TypedContext, Type );
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, nullptr, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ), typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

template < CStatefulWithContextType type_t, CStatefulWithContextType other_type_t, class owner_t >
	requires SFstd::is_mutable_pointer< typename type_t::ContextType* > && SFstd::derived_from< type_t, other_type_t >
FMessageListenerHandle UStarfireMessenger::StartListeningForMessage( const owner_t *Owner, void (owner_t::* Callback)( const TConstStructView< other_type_t >&, typename type_t::ContextType*, EStatefulMessageEvent ) const, typename type_t::ContextType *ContextFilter )
{
	const auto MessageType = type_t::StaticStruct( );
	auto MutableFilter = const_cast< std::remove_cv_t< typename type_t::ContextType >* >( ContextFilter );
	TWeakObjectPtr< const owner_t > WeakOwner( Owner );

	auto BoxedCallback = [ WeakOwner, Callback ]( const FConstStructView &View, UObject* Context, EStatefulMessageEvent Type ) -> void
	{
		if (const auto StrongOwner = WeakOwner.Get())
		{
			TConstStructView< other_type_t > TypedView;
			TypedView.SetStructData( View.GetScriptStruct( ), View.GetMemory( ) );

			const auto TypedContext = CastChecked< typename type_t::ContextType >( Context );

			(StrongOwner->*Callback)( TypedView, TypedContext, Type );
		}
	};
	return StartListeningForStatefulMessageInternal( MessageType, BoxedCallback, Owner, MutableFilter, true );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CStatefulNoContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::ClearStatefulMessage( )
{
	const auto MessageType = type_t::StaticStruct( );
	ClearStatefulMessage( false, MessageType, nullptr );
}

template < CStatefulWithContextType type_t >
	requires (!CAbstractMessageType< type_t >)
void UStarfireMessenger::ClearStatefulMessage( typename type_t::ContextType *Context )
{
	const auto MessageType = type_t::StaticStruct( );
	ClearStatefulMessage( true, MessageType, Context );
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
template < CStatefulNoContextType type_t >
	requires (!CAbstractMessageType< type_t >)
bool UStarfireMessenger::HasStatefulMessage() const
{
	const auto MessageType = type_t::StaticStruct( );
	return HasStatefulMessage( false, MessageType, nullptr );
}

template < CStatefulWithContextType type_t >
	requires (!CAbstractMessageType< type_t >)
bool UStarfireMessenger::HasStatefulMessage( typename type_t::ContextType *Context ) const
{
	const auto MessageType = type_t::StaticStruct( );
	return HasStatefulMessage( true, MessageType, Context );
}
