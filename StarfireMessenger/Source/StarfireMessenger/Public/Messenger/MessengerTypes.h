
#pragma once

#include "UObject/ObjectMacros.h"

#include "MessengerTypes.generated.h"

// The reason that stateful message data is being broadcast
UENUM( )
enum class EStatefulMessageEvent : uint8
{
	// A new stateful message is being broadcast
	NewMessage,

	// Messenger is providing an existing message at the time of message registration
	ExistingMessage,

	// A stateful message is being cleared so that will no longer be valid state
	Clearing,
};

// Strongly typed wrapper around a value for uniquely identifying a specific listener at a later time
USTRUCT( BlueprintType )
struct FMessageListenerHandle
{
	GENERATED_BODY( )
public:
	// Default constructors and assignment are find
	FMessageListenerHandle( void ) = default;
	FMessageListenerHandle( const FMessageListenerHandle & ) = default;
	FMessageListenerHandle& operator=( const FMessageListenerHandle & ) = default;

	// Function to check if this handle should be a reference to something
	[[nodiscard]] bool IsValid( void ) const { return Handle > 0; }
	// Function to make this handle no longer a reference to anything
	void Invalidate( void ) { Handle = 0; }

	// Equality checking operators
	[[nodiscard]] bool operator==( const FMessageListenerHandle &rhs ) const { return Handle == rhs.Handle; }
	[[nodiscard]] bool operator!=( const FMessageListenerHandle &rhs ) const { return Handle != rhs.Handle; }

	// Accessor to get at an integer representation of the handle for various debug purposes
	[[nodiscard]] int AsInt( void ) const { return Handle; }

	// Support Sets and as a Map key
	friend uint32 GetTypeHash( const FMessageListenerHandle &H )
	{
		return GetTypeHash( H.Handle );
	}

private:

	// The value of the unique identifier
	UPROPERTY( )
	int	Handle = 0;

	// Non-default constructor and friend, only the Event Manager should be creating these handles
	explicit FMessageListenerHandle( int id ): Handle( id ) { }
	friend class UStarfireMessenger;
};