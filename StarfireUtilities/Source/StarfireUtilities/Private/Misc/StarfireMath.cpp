
#include "Misc/StarfireMath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(StarfireMath)

int UStarfireMath::RoundToInt( float A, ERoundingStyle Style )
{
	switch (Style)
	{
		case ERoundingStyle::FromZero:
			return FMath::RoundFromZero( A );

		case ERoundingStyle::HalfFromZero:
			return FMath::RoundHalfFromZero( A );

		case ERoundingStyle::HalfToEven:
			return FMath::RoundHalfToEven( A );

		case ERoundingStyle::HalfToZero:
			return FMath::RoundHalfToZero( A );

		case ERoundingStyle::ToNegativeInfinity:
			return FMath::RoundToNegativeInfinity( A );

		case ERoundingStyle::ToPositiveInfinity:
			return FMath::RoundToPositiveInfinity( A );

		case ERoundingStyle::ToZero:
			return FMath::RoundToZero( A );
	}

	checkNoEntry( );
	return 0;
}

float UStarfireMath::RoundToFloat( float A, ERoundingStyle Style )
{
	switch (Style)
	{
		case ERoundingStyle::FromZero:
			return FMath::RoundFromZero( A );

		case ERoundingStyle::HalfFromZero:
			return FMath::RoundHalfFromZero( A );

		case ERoundingStyle::HalfToEven:
			return FMath::RoundHalfToEven( A );

		case ERoundingStyle::HalfToZero:
			return FMath::RoundHalfToZero( A );

		case ERoundingStyle::ToNegativeInfinity:
			return FMath::RoundToNegativeInfinity( A );

		case ERoundingStyle::ToPositiveInfinity:
			return FMath::RoundToPositiveInfinity( A );

		case ERoundingStyle::ToZero:
			return FMath::RoundToZero( A );
	}

	checkNoEntry( );
	return 0;
}