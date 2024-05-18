
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StarfireMath.generated.h"

UENUM( )
enum class ERoundingStyle : uint8
{
	/** Rounds to the nearest place, equidistant ties go to the value which is closest to an even value: 1.5 becomes 2, 0.5 becomes 0 */
	HalfToEven,
	/** Rounds to nearest place, equidistant ties go to the value which is further from zero: -0.5 becomes -1.0, 0.5 becomes 1.0 */
	HalfFromZero,
	/** Rounds to nearest place, equidistant ties go to the value which is closer to zero: -0.5 becomes 0, 0.5 becomes 0. */
	HalfToZero,
	/** Rounds to the value which is further from zero, "larger" in absolute value: 0.1 becomes 1, -0.1 becomes -1 */
	FromZero,
	/** Rounds to the value which is closer to zero, "smaller" in absolute value: 0.1 becomes 0, -0.1 becomes 0 */
	ToZero,
	/** Rounds to the value which is more negative: 0.1 becomes 0, -0.1 becomes -1 */
	ToNegativeInfinity,
	/** Rounds to the value which is more positive: 0.1 becomes 1, -0.1 becomes 0 */
	ToPositiveInfinity,
};

UCLASS( )
class STARFIREUTILITIES_API UStarfireMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY( )
public:
	// Round a float to the nearest integer
	UFUNCTION( BlueprintCallable, BlueprintPure = true, Category = "Core|Math" )
	static int RoundToInt( float A, ERoundingStyle Style );
	
	// Round a float to the nearest integer but keep it as a floating type
	UFUNCTION( BlueprintCallable, BlueprintPure = true, Category = "Core|Math" )
	static float RoundToFloat( float A, ERoundingStyle Style );
};