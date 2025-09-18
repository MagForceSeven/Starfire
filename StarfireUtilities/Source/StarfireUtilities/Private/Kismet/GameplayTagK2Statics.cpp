
#include "GameplayTagK2Statics.h"

#include "GameplayTagContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagK2Statics)

int UGameplayTagK2Statics::DetermineBestMatch( FGameplayTag CheckTag, const TArray< FGameplayTag > &Tags )
{
	int BestMatchIndex = -1;
	int BestMatchDepth = -1;

	for (int idx = 0; idx < Tags.Num(); idx++)
	{
		const auto &Tag = Tags[ idx ];

		const auto TagDepth = Tag.GetGameplayTagParents( ).Num( );
		const auto MatchDepth = CheckTag.MatchesTagDepth( Tag );

		if (TagDepth != MatchDepth)
			continue; // skip if we don't match the whole tag

		if (MatchDepth <= BestMatchDepth)
			continue; // skip if it's not as good as current match

		BestMatchIndex = idx;
		BestMatchDepth = MatchDepth;
	}

	return BestMatchIndex;
}
