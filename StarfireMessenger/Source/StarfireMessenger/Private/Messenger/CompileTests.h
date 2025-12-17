
#pragma once

#include "GameFramework/Actor.h"

#include "Messenger/MessageTypes.h"
#include "StructUtils/StructView.h"

#include "CompileTests.generated.h"

struct FMessageTest;
struct FStatefulTest;
struct FMessageTest_Const;
struct FStatefulTest_Const;
struct FMessageTest_NoContext;
struct FStatefulTest_NoContext;

struct FMessageTest_C;
struct FStatefulTest_C;
struct FMessageTest_Const_C;
struct FStatefulTest_Const_C;
struct FMessageTest_NoContext_C;
struct FStatefulTest_NoContext_C;

enum class EStatefulMessageEvent : uint8;

//
UCLASS( Abstract, NotBlueprintType, NotBlueprintable )
class AMessengerCompileTests : public AActor
{
	GENERATED_BODY( )
public:
	// Actor API
	void BeginPlay( ) override;

	void Handler( const FMessageTest&, AMessengerCompileTests* ) { }
	void ConstHandler( const FMessageTest&, AMessengerCompileTests* ) const { }
	void ConstContextHandler( const FMessageTest&, const AMessengerCompileTests* ) { }
	void ConstContextConstHandler( const FMessageTest&, const AMessengerCompileTests* ) const { }
	static void StaticHandler( const FMessageTest&, AMessengerCompileTests* ) { }
	static void StaticConstContextHandler( const FMessageTest&, const AMessengerCompileTests* ) { }
	void HandlerC( const FMessageTest_C&, AMessengerCompileTests* ) { }
	void ConstContextHandlerC( const FMessageTest_C&, const AMessengerCompileTests* ) { }

	void Handler_H( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) { }
	void ConstHandler_H( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) const { }
	void ConstContextHandler_H( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) { }
	void ConstContextConstHandler_H( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) const { }
	static void StaticHandler_H( const TConstStructView< FMessageTest >&, AMessengerCompileTests* ) { }
	static void StaticConstContextHandler_H( const TConstStructView< FMessageTest >&, const AMessengerCompileTests* ) { }
	void HandlerC_H( const TConstStructView< FMessageTest_C >&, AMessengerCompileTests* ) { }
	void ConstContextHandlerC_H( const TConstStructView< FMessageTest_C >&, const AMessengerCompileTests* ) { }

	void Stateful_Handler( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstHandler( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	void Stateful_ConstContextHandler( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextConstHandler( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	static void Stateful_StaticHandler( const FStatefulTest&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	static void Stateful_StaticConstContextHandler( const FStatefulTest&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_HandlerC( const FStatefulTest_C&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextHandlerC( const FStatefulTest_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }

	void Stateful_Handler_H( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstHandler_H( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	void Stateful_ConstContextHandler_H( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextConstHandler_H( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	static void Stateful_StaticHandler_H( const TConstStructView< FStatefulTest >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	static void Stateful_StaticConstContextHandler_H( const TConstStructView< FStatefulTest >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_HandlerC_H( const TConstStructView< FStatefulTest_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextHandlerC_H( const TConstStructView< FStatefulTest_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }

	void Handler_C( const FMessageTest_Const&, AMessengerCompileTests* ) { }
	void ConstHandler_C( const FMessageTest_Const&, AMessengerCompileTests* ) const { }
	void ConstContextHandler_C( const FMessageTest_Const&, const AMessengerCompileTests* ) { }
	void ConstContextConstHandler_C( const FMessageTest_Const&, const AMessengerCompileTests* ) const { }
	static void StaticHandler_C( const FMessageTest_Const&, AMessengerCompileTests* ) { }
	static void StaticConstContextHandler_C( const FMessageTest_Const&, const AMessengerCompileTests* ) { }
	void HandlerC_C( const FMessageTest_Const_C&, AMessengerCompileTests* ) { }
	void ConstContextHandlerC_C( const FMessageTest_Const_C&, const AMessengerCompileTests* ) { }

	void Handler_H_C( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) { }
	void ConstHandler_H_C( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) const { }
	void ConstContextHandler_H_C( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) { }
	void ConstContextConstHandler_H_C( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) const { }
	static void StaticHandler_H_C( const TConstStructView< FMessageTest_Const >&, AMessengerCompileTests* ) { }
	static void StaticConstContextHandler_H_C( const TConstStructView< FMessageTest_Const >&, const AMessengerCompileTests* ) { }
	void HandlerC_H_C( const TConstStructView< FMessageTest_Const_C >&, AMessengerCompileTests* ) { }
	void ConstContextHandlerC_H_C( const TConstStructView< FMessageTest_Const_C >&, const AMessengerCompileTests* ) { }

	void Stateful_Handler_C( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstHandler_C( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	void Stateful_ConstContextHandler_C( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextConstHandler_C( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	static void Stateful_StaticHandler_C( const FStatefulTest_Const&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	static void Stateful_StaticConstContextHandler_C( const FStatefulTest_Const&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_HandlerC_C( const FStatefulTest_Const_C&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextHandlerC_C( const FStatefulTest_Const_C&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }

	void Stateful_Handler_H_C( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstHandler_H_C( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	void Stateful_ConstContextHandler_H_C( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextConstHandler_H_C( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) const { }
	static void Stateful_StaticHandler_H_C( const TConstStructView< FStatefulTest_Const >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	static void Stateful_StaticConstContextHandler_H_C( const TConstStructView< FStatefulTest_Const >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_HandlerC_H_C( const TConstStructView< FStatefulTest_Const_C >&, AMessengerCompileTests*, EStatefulMessageEvent ) { }
	void Stateful_ConstContextHandlerC_H_C( const TConstStructView< FStatefulTest_Const_C >&, const AMessengerCompileTests*, EStatefulMessageEvent ) { }

	void NC_Handler( const FMessageTest_NoContext& ) { }
	void NC_ConstHandler( const FMessageTest_NoContext& ) const { }
	static void NC_StaticHandler( const FMessageTest_NoContext& ) { }
	void NC_HandlerC( const FMessageTest_NoContext_C& ) { }

	void NC_Handler_H( const TConstStructView< FMessageTest_NoContext >& ) { }
	void NC_ConstHandler_H( const TConstStructView< FMessageTest_NoContext >& ) const { }
	static void NC_StaticHandler_H( const TConstStructView< FMessageTest_NoContext >& ) { }
	void NC_HandlerC_H( const TConstStructView< FMessageTest_NoContext_C >& ) { }

	void NC_Stateful_Handler( const FStatefulTest_NoContext&, EStatefulMessageEvent ) { }
	void NC_Stateful_ConstHandler( const FStatefulTest_NoContext&, EStatefulMessageEvent ) const { }
	static void NC_Stateful_StaticHandler( const FStatefulTest_NoContext&, EStatefulMessageEvent ) { }
	void NC_Stateful_HandlerC( const FStatefulTest_NoContext_C&, EStatefulMessageEvent ) { }

	void NC_Stateful_Handler_H( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) { }
	void NC_Stateful_ConstHandler_H( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) const { }
	static void NC_Stateful_StaticHandler_H( const TConstStructView< FStatefulTest_NoContext >&, EStatefulMessageEvent ) { }
	void NC_Stateful_HandlerC_H( const TConstStructView< FStatefulTest_NoContext_C >&, EStatefulMessageEvent ) { }
};

USTRUCT( meta = (Hidden) )
struct FMessageTest : public FSf_Message_Immediate
{
	GENERATED_BODY( )
public:

	SET_CONTEXT_TYPE( AMessengerCompileTests )
};

USTRUCT( meta = (Hidden) )
struct FMessageTest_C : public FMessageTest
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest : public FSf_Message_Stateful
{
	GENERATED_BODY( )
public:

	SET_CONTEXT_TYPE( AMessengerCompileTests )
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest_C : public FStatefulTest
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FMessageTest_Const : public FSf_Message_Immediate
{
	GENERATED_BODY( )
public:

	SET_CONTEXT_TYPE( const AMessengerCompileTests )
};

USTRUCT( meta = (Hidden) )
struct FMessageTest_Const_C : public FMessageTest_Const
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest_Const : public FSf_Message_Stateful
{
	GENERATED_BODY( )
public:

	SET_CONTEXT_TYPE( const AMessengerCompileTests )
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest_Const_C : public FStatefulTest_Const
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FMessageTest_NoContext : public FSf_Message_Immediate
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FMessageTest_NoContext_C : public FMessageTest_NoContext
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest_NoContext : public FSf_Message_Stateful
{
	GENERATED_BODY( )
public:
};

USTRUCT( meta = (Hidden) )
struct FStatefulTest_NoContext_C : public FStatefulTest_NoContext
{
	GENERATED_BODY( )
public:
};