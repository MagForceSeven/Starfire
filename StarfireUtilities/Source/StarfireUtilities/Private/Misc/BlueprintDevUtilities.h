
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BlueprintDevUtilities.generated.h"

// Types of builds that may be running
UENUM( )
enum class EBuildType : uint8
{
	// Non-user build for developers only
	Development,

	// Non-user build close to shipping for testing
	Testing,

	// User build that could be shipped to customers
	Shipping,
};

UCLASS( )
class UBlueprintDevUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:

	// A non fatal, but halting error that will also be output to the log
	UFUNCTION( BlueprintCallable, Category = "Dev Utilities", meta = (DevelopmentOnly, AutoCreateRefTerm = "Message") )
	static void Ensure( bool Condition, const FString &Message );

	// A fatal and halting error
	UFUNCTION( BlueprintCallable, Category = "Dev Utilities", meta = (DevelopmentOnly, AutoCreateRefTerm = "Message") )
	static void Check( bool Condition, const FString &Message );

	// Determine the current type of build that is running (since blueprint doesn't have another good way for conditional compiles)
	UFUNCTION( BlueprintCallable, Category = "Dev Utilities", BlueprintPure )
	[[nodiscard]] static EBuildType GetCurrentBuildType( );

	// Branch the execution based on the current type of build that is running (since blueprint doesn't have another good way for conditional compiles)
	UFUNCTION( BlueprintCallable, Category = "Dev Utilities", meta = (ExpandEnumAsExecs = Exec) )
	static void SwitchOnBuildType( EBuildType &Exec );
};