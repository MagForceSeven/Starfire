
#pragma once

#include "View/MVVMViewModelContextResolver.h"

#include "ActorVMResolver.generated.h"

class UActorVMBase;

// A resolver that can be used to bind to a VM of the associated Actor
UCLASS( DisplayName = "Actor VM Resolver" )
class ACTORVIEWMODELS_API UActorVMResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY( )
public:
	// MVVM View Model Context Resolver API
	UObject* CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const override;
	
#if WITH_EDITOR
	bool DoesSupportViewModelClass( const UClass* Class ) const override;
#endif

protected:
	// Utility to give derived classes access to a Find/Create API with these expectedly available parameters
	[[nodiscard]] static UActorVMBase* FindOrCreateVM( AActor *Actor, const UClass *Class );
};

// A resolver that can be used to bind to a VM of an Actor there should only ever be a single instance of
UCLASS( DisplayName = "Singleton Actor VM Resolver" )
class ACTORVIEWMODELS_API USingletonActorVMResolver : public UActorVMResolver
{
	GENERATED_BODY( )
public:
	// MVVM View Model Context Resolver API
	UObject* CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const override;

	// The type of actor instance to access the VMs of
	UPROPERTY( EditDefaultsOnly )
	TSubclassOf< AActor > SingletonActorClass;
};

// A resolver that can be used to bind to an Actor VM that there should only ever be a single instance of
UCLASS( DisplayName = "Singleton VM Resolver" )
class ACTORVIEWMODELS_API USingletonVMResolver : public UActorVMResolver
{
	GENERATED_BODY( )
public:
	// MVVM View Model Context Resolver API
	UObject* CreateInstance( const UClass *ExpectedType, const UUserWidget *UserWidget, const UMVVMView *View ) const override;
	
#if WITH_EDITOR
	bool DoesSupportViewModelClass( const UClass* Class ) const override;
#endif
};