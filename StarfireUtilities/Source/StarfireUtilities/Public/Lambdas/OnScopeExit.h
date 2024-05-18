
#pragma once

#include "Misc/ScopeExit.h"

#define SF_ON_SCOPE_EXIT( Suffix ) const auto PREPROCESSOR_JOIN( PREPROCESSOR_JOIN( ScopeGuard_, __LINE__ ), Suffix ) = ::ScopeExitSupport::FScopeGuardSyntaxSupport( ) + [ & ]( )