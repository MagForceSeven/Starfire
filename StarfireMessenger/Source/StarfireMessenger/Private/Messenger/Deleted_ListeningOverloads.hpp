
// ReSharper disable once CppMissingIncludeGuard
#ifndef STARFIRE_MESSENGER_HPP
	#error You shouldn't be including this file directly
#endif

	// Deleted & deprecated versions of the Listening functions to produce better error results than a bunch of non-matching overloads
	// Deleted to produce the most helpful error
	// Deprecated to also produce a more specific user error for why it is disallowed
public:
	template < CImmediateNoContextType type_t, CImmediateNoContextType other_type_t >
		requires (SFstd::derived_from< other_type_t, type_t > && !std::is_same_v< type_t, other_type_t >)
	[[deprecated("Attempting to handle message using child message type. This is unsafe as other children of message type may be possible.")]]
	FMessageListenerHandle StartListeningForMessage( const UObject *Owner, TFunction< void ( const other_type_t& )> &&Callback ) = delete;

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
private: