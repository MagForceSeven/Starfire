
#include "Messenger/CompileTests.h"

#include "Messenger/Messenger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CompileTests)

#define SFM_CHECK_ERRORS 0

void AMessengerCompileTests::BeginPlay( )
{
	Super::BeginPlay( );

	const auto Router = UStarfireMessenger::GetSubsystem( this );

#if SFM_CHECK_ERRORS
	// Can't broadcast abstract instances
	Router->Broadcast< FBroadcastTest_Abstract >( { } );
	Router->Broadcast< FBroadcastTest_Context_Abstract >( { }, this );
	Router->Broadcast< FBroadcastTest_Abstract_Stateful >( { } );
	Router->Broadcast< FBroadcastTest_Context_Abstract_Stateful >( { }, this );

	// Can't broadcast with matching constructor
	Router->Broadcast< FBroadcastTest_Abstract >( );
	Router->Broadcast< FBroadcastTest_Context_Abstract >( this );
	Router->Broadcast< FBroadcastTest_Abstract_Stateful >( );
	Router->Broadcast< FBroadcastTest_Context_Abstract_Stateful >( this );

	// Mismatch message type & Context data - but also abstract
	Router->Broadcast< FBroadcastTest_Abstract >( this );
	Router->Broadcast< FBroadcastTest_Context_Abstract >( );
	Router->Broadcast< FBroadcastTest_Abstract_Stateful >( this );
	Router->Broadcast< FBroadcastTest_Context_Abstract_Stateful >( );

	// Mismatch message type & Context input
	Router->Broadcast< FBroadcastTest >( this );
	Router->Broadcast< FBroadcastTest_Context >( );
	Router->Broadcast< FBroadcastTest_Stateful >( this );
	Router->Broadcast< FBroadcastTest_Context_Stateful >( );

	// Mismatch message type & Context input
	Router->Broadcast< FBroadcastTest >( { }, this );
	Router->Broadcast< FBroadcastTest_Context >( FBroadcastTest_Context( ) );
	Router->Broadcast< FBroadcastTest_Stateful >( { }, this );
	Router->Broadcast< FBroadcastTest_Context_Stateful >( FBroadcastTest_Context_Stateful( ) );

	// Can't clear abstract message types
	Router->ClearStatefulMessage< FBroadcastTest_Abstract_Stateful >( );
	Router->ClearStatefulMessage< FBroadcastTest_Abstract_Stateful >( this );
	Router->ClearStatefulMessage< FBroadcastTest_Context_Abstract_Stateful >( );
	Router->ClearStatefulMessage< FBroadcastTest_Context_Abstract_Stateful >( this );

	// Mismatch message type & Context input
	Router->ClearStatefulMessage< FBroadcastTest_Stateful >( this );
	Router->ClearStatefulMessage< FBroadcastTest_Context_Stateful >( );

	// Can't check stateful status for abstract types
	Router->HasStatefulMessage< FBroadcastTest_Abstract_Stateful >( );
	Router->HasStatefulMessage< FBroadcastTest_Abstract_Stateful >( this );
	Router->HasStatefulMessage< FBroadcastTest_Context_Abstract_Stateful >( );
	Router->HasStatefulMessage< FBroadcastTest_Context_Abstract_Stateful >( this );
	
	// Mismatch message type & Context input
	Router->HasStatefulMessage< FBroadcastTest_Stateful >( this );
	Router->HasStatefulMessage< FBroadcastTest_Context_Stateful >( );
#endif

	Router->Broadcast< FBroadcastTest >( );
	Router->Broadcast< FBroadcastTest_Context >( this );
	Router->Broadcast< FBroadcastTest_Stateful >( );
	Router->Broadcast< FBroadcastTest_Context_Stateful >( this );
	
	{
		FBroadcastTest Test;
		Router->Broadcast< FBroadcastTest >( Test );
	}

	{
		FBroadcastTest_Context Test;
		Router->Broadcast< FBroadcastTest_Context >( Test, this  );
	}

	{
		FBroadcastTest_Stateful Test;
		Router->Broadcast< FBroadcastTest_Stateful >( Test );
	}

	{
		FBroadcastTest_Context_Stateful Test;
		Router->Broadcast< FBroadcastTest_Context_Stateful >( Test, this  );
	}

	Router->Broadcast< FBroadcastTest_Constructor >( 42, 3.14f );
	Router->Broadcast< FBroadcastTest_Context_Constructor >( this, 42, 3.14f );
	Router->Broadcast< FBroadcastTest_Stateful_Constructor >( 42, 3.14f );
	Router->Broadcast< FBroadcastTest_Context_Stateful_Constructor >( this, 42, 3.14f );

	Router->Broadcast< FBroadcastTest_Initializer >( { .i = 42, .f = 3.14f } );
	Router->Broadcast< FBroadcastTest_Context_Initializer >( { .i = 42, .f = 3.14f }, this );
	Router->Broadcast< FBroadcastTest_Stateful_Initializer >( { .i = 42, .f = 3.14f } );
	Router->Broadcast< FBroadcastTest_Context_Stateful_Initializer >( { .i = 42, .f = 3.14f }, this );
	
	Router->ClearStatefulMessage< FBroadcastTest_Stateful >( );
	Router->ClearStatefulMessage< FBroadcastTest_Context_Stateful >( this );

	Router->HasStatefulMessage< FBroadcastTest_Stateful >( );
	Router->HasStatefulMessage< FBroadcastTest_Context_Stateful >( this );

	FBroadcastTest_Mixed Test = FBroadcastTest_Mixed( FString( ) );
	Router->Broadcast< FBroadcastTest_Mixed >( Test );
	//FBroadcastTest_Mixed Test2 = { .j = 42, .f = 3.14f };
/*
	Router->Broadcast< FBroadcastTest_Mixed >( { .j = 42, .f = 3.14f } );
	Router->Broadcast< FBroadcastTest_Context_Mixed >( { .j = 42, .f = 3.14f }, this );
	Router->Broadcast< FBroadcastTest_Stateful_Mixed >( { .j = 42, .f = 3.14f } );
	Router->Broadcast< FBroadcastTest_Context_Stateful_Mixed >( { .j = 42, .f = 3.14f }, this );
*/

	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::Handler );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstHandler );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextHandler );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextConstHandler );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const FMessageTest&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const FMessageTest&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( &AMessengerCompileTests::StaticHandler );
	Router->StartListeningForMessage< FMessageTest >( &AMessengerCompileTests::StaticConstContextHandler );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const FMessageTest&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const FMessageTest&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::StaticHandler );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::StaticConstContextHandler );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::HandlerC );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstHandlerC );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextHandlerC );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextConstHandlerC );
#endif

	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::Handler_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstHandler_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextHandler_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextConstHandler_H );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( &AMessengerCompileTests::StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest >( &AMessengerCompileTests::StaticConstContextHandler_H );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::StaticConstContextHandler_H );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::HandlerC_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstHandlerC_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextHandlerC_H );
	Router->StartListeningForMessage< FMessageTest >( this, &AMessengerCompileTests::ConstContextConstHandlerC_H );
#endif

	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::Handler );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstHandler );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextHandler );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextConstHandler );
	Router->StartListeningForMessage< FMessageTest_C >( [ ]( const FMessageTest&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( [ ]( const FMessageTest&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( &AMessengerCompileTests::StaticHandler );
	Router->StartListeningForMessage< FMessageTest_C >( &AMessengerCompileTests::StaticConstContextHandler );
	Router->StartListeningForMessage< FMessageTest_C >( this, [ ]( const FMessageTest&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( this, [ ]( const FMessageTest&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::StaticHandler );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::StaticConstContextHandler );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::HandlerC );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstHandlerC );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextHandlerC );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextConstHandlerC );
	
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest >( [ ]( const FMessageTest_C&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const FMessageTest_C&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const FMessageTest_C&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const FMessageTest_C&, const AMessengerCompileTests* ) -> void { } );
#endif

	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::Handler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextConstHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( [ ]( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( [ ]( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( &AMessengerCompileTests::StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( &AMessengerCompileTests::StaticConstContextHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, [ ]( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( this, [ ]( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::StaticConstContextHandler_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::HandlerC_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstHandlerC_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextHandlerC_H );
	Router->StartListeningForMessage< FMessageTest_C >( this, &AMessengerCompileTests::ConstContextConstHandlerC_H );

	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest >( [ ]( const TConstStructView< FMessageTest_C >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( [ ]( const TConstStructView< FMessageTest_C >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const TConstStructView< FMessageTest_C >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest >( this, [ ]( const TConstStructView< FMessageTest_C >&, const AMessengerCompileTests* ) -> void { } );
#endif

	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_Handler );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstHandler );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextHandler );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( &AMessengerCompileTests::Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest >( &AMessengerCompileTests::Stateful_StaticConstContextHandler );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_HandlerC );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstHandlerC );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC );
#endif

	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_Handler_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( &AMessengerCompileTests::Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_H );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_HandlerC_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_H );
	Router->StartListeningForMessage< FStatefulTest >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_H );
#endif

	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_Handler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( [ ]( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( [ ]( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( &AMessengerCompileTests::Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( &AMessengerCompileTests::Stateful_StaticConstContextHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, [ ]( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( this, [ ]( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_HandlerC );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstHandlerC );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC );

	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const FStatefulTest_C&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const FStatefulTest_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const FStatefulTest_C&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const FStatefulTest_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
#endif

	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_Handler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( [ ]( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( [ ]( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( &AMessengerCompileTests::Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, [ ]( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( this, [ ]( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_HandlerC_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_H );
	Router->StartListeningForMessage< FStatefulTest_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_H );

	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const TConstStructView< FStatefulTest_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( [ ]( const TConstStructView< FStatefulTest_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const TConstStructView< FStatefulTest_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest >( this, [ ]( const TConstStructView< FStatefulTest_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::Handler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const FMessageTest_Const&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( &AMessengerCompileTests::StaticHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const FMessageTest_Const&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::StaticHandler_C );
#endif
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextConstHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const FMessageTest_Const&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( &AMessengerCompileTests::StaticConstContextHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const FMessageTest_Const&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::StaticConstContextHandler_C );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::HandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstHandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextHandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextConstHandlerC_C );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::Handler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( &AMessengerCompileTests::StaticHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::StaticHandler_H_C );
#endif
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextConstHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( &AMessengerCompileTests::StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::StaticConstContextHandler_H_C );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::HandlerC_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstHandlerC_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextHandlerC_H_C );
	Router->StartListeningForMessage< FMessageTest_Const >( this, &AMessengerCompileTests::ConstContextConstHandlerC_H_C );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::Handler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( [ ]( const FMessageTest_Const&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, [ ]( const FMessageTest_Const&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::HandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstHandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( &AMessengerCompileTests::StaticHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::StaticHandler_C );
#endif
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextConstHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( [ ]( const FMessageTest_Const&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( &AMessengerCompileTests::StaticConstContextHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, [ ]( const FMessageTest_Const&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::StaticConstContextHandler_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextHandlerC_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextConstHandlerC_C );

	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const FMessageTest_Const_C&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const FMessageTest_Const_C&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const FMessageTest_Const_C&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const FMessageTest_Const_C&, const AMessengerCompileTests* ) -> void { } );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::Handler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( [ ]( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( &AMessengerCompileTests::StaticHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, [ ]( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::StaticHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::HandlerC_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstHandlerC_H_C );
#endif
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextConstHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( [ ]( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( &AMessengerCompileTests::StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, [ ]( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextHandlerC_H_C );
	Router->StartListeningForMessage< FMessageTest_Const_C >( this, &AMessengerCompileTests::ConstContextConstHandlerC_H_C );

	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const TConstStructView< FMessageTest_Const_C >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( [ ]( const TConstStructView< FMessageTest_Const_C >&, const AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const TConstStructView< FMessageTest_Const_C >&, AMessengerCompileTests* ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_Const >( this, [ ]( const TConstStructView< FMessageTest_Const_C >&, const AMessengerCompileTests* ) -> void { } );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_Handler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( &AMessengerCompileTests::Stateful_StaticHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_StaticHandler_C );
#endif
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_C );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_HandlerC_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_C );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_Handler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( &AMessengerCompileTests::Stateful_StaticHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_StaticHandler_H_C );
#endif
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_H_C );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_HandlerC_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_H_C );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_Handler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( [ ]( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( &AMessengerCompileTests::Stateful_StaticHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, [ ]( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_StaticHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_HandlerC_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_C );
#endif
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( [ ]( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, [ ]( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_C );
	
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const FStatefulTest_Const_C&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const FStatefulTest_Const_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const FStatefulTest_Const_C&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const FStatefulTest_Const_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
#endif

	// Message declares const Context type, mutable handler context params not allowed
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_Handler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( [ ]( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( &AMessengerCompileTests::Stateful_StaticHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, [ ]( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_StaticHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_HandlerC_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstHandlerC_H_C );
#endif
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( [ ]( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( &AMessengerCompileTests::Stateful_StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, [ ]( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_StaticConstContextHandler_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextHandlerC_H_C );
	Router->StartListeningForMessage< FStatefulTest_Const_C >( this, &AMessengerCompileTests::Stateful_ConstContextConstHandlerC_H_C );

	// Can't listen to parent type and expect a child type	
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const TConstStructView< FStatefulTest_Const_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( [ ]( const TConstStructView< FStatefulTest_Const_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const TConstStructView< FStatefulTest_Const_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_Const >( this, [ ]( const TConstStructView< FStatefulTest_Const_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) -> void { } );
#endif

	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_Handler );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_ConstHandler );
	Router->StartListeningForMessage< FMessageTest_NoContext >( [ ]( const FMessageTest_NoContext& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( &AMessengerCompileTests::NC_StaticHandler );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, [ ]( const FMessageTest_NoContext& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_StaticHandler );
	// Can't listen to parent type and expect a child type	
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_HandlerC );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_ConstHandlerC );
#endif

	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_Handler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_ConstHandler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext >( [ ]( const TConstStructView< FMessageTest_NoContext >& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( &AMessengerCompileTests::NC_StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, [ ]( const TConstStructView< FMessageTest_NoContext >& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_StaticHandler_H );
	// Can't listen to parent type and expect a child type	
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_HandlerC_H );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, &AMessengerCompileTests::NC_ConstHandlerC_H );
#endif

	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_Handler );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_ConstHandler );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( [ ]( const FMessageTest_NoContext& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( &AMessengerCompileTests::NC_StaticHandler );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, [ ]( const FMessageTest_NoContext& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_StaticHandler );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_NoContext >( [ ]( const FMessageTest_NoContext_C& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, [ ]( const FMessageTest_NoContext_C& ) -> void { } );
#endif
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_HandlerC );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_ConstHandlerC );

	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_Handler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_ConstHandler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( [ ]( const TConstStructView< FMessageTest_NoContext >& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( &AMessengerCompileTests::NC_StaticHandler_H );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, [ ]( const TConstStructView< FMessageTest_NoContext >& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_StaticHandler_H );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FMessageTest_NoContext >( [ ]( const TConstStructView< FMessageTest_NoContext_C >& ) -> void { } );
	Router->StartListeningForMessage< FMessageTest_NoContext >( this, [ ]( const TConstStructView< FMessageTest_NoContext_C >& ) -> void { } );
#endif
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_HandlerC_H );
	Router->StartListeningForMessage< FMessageTest_NoContext_C >( this, &AMessengerCompileTests::NC_ConstHandlerC_H );

	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_Handler );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_ConstHandler );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( [ ]( const FStatefulTest_NoContext&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( &AMessengerCompileTests::NC_Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, [ ]( const FStatefulTest_NoContext&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_StaticHandler );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_HandlerC );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_ConstHandlerC );
#endif

	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_Handler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_ConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( [ ]( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( &AMessengerCompileTests::NC_Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, [ ]( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_StaticHandler_H );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_HandlerC_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, &AMessengerCompileTests::NC_Stateful_ConstHandlerC_H );
#endif

	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_Handler );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_ConstHandler );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( [ ]( const FStatefulTest_NoContext&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( &AMessengerCompileTests::NC_Stateful_StaticHandler );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, [ ]( const FStatefulTest_NoContext&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_StaticHandler );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_NoContext >( [ ]( const FStatefulTest_NoContext_C&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, [ ]( const FStatefulTest_NoContext_C&, EStatefulMessageEvent ) -> void { } );
#endif
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_HandlerC );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_ConstHandlerC );

	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_Handler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_ConstHandler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( [ ]( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( &AMessengerCompileTests::NC_Stateful_StaticHandler_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, [ ]( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_StaticHandler_H );
	// Can't listen to parent type and expect a child type
#if SFM_CHECK_ERRORS
	Router->StartListeningForMessage< FStatefulTest_NoContext >( [ ]( const TConstStructView< FStatefulTest_NoContext_C >&, EStatefulMessageEvent ) -> void { } );
	Router->StartListeningForMessage< FStatefulTest_NoContext >( this, [ ]( const TConstStructView< FStatefulTest_NoContext_C >&, EStatefulMessageEvent ) -> void { } );
#endif
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_HandlerC_H );
	Router->StartListeningForMessage< FStatefulTest_NoContext_C >( this, &AMessengerCompileTests::NC_Stateful_ConstHandlerC_H );
}
