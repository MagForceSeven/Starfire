
#pragma once

#include "NativeGameplayTags.h"

// Use UE_DEFINE_GAMEPLAY_TAG in the cpp like: UE_DEFINE_GAMEPLAY_TAG( Class::StaticVarName, <tag> )
#define UE_DECLARE_GAMEPLAY_TAG_SCOPED(TagName) static FNativeGameplayTag TagName;
