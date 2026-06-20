
#pragma once

#include "Misc/ScopeExit.h"

#define SF_ON_SCOPE_EXIT( Suffix ) const auto UE_JOIN( UE_JOIN( ScopeGuard_, __LINE__ ), Suffix ) = ::ScopeExitSupport::FScopeGuardSyntaxSupport( ) + [ & ]( )