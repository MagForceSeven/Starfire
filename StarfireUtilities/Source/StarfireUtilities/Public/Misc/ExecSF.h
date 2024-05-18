
#pragma once

#include "Misc/CoreMisc.h"
#include "Delegates/Delegate.h"

class UWorld;
class FOutputDevice;

namespace ExecSF_Params
{
	// Template utility for parsing command parameters
	template < typename First_t, typename... Args_t >
	int GetParams( const TCHAR *&Cmd, First_t &First, Args_t&... Args );

	template < typename type_t >
	int GetParams( const TCHAR *&Cmd, type_t &output ) { checkNoEntry( ); return 0; }

	// Concrete utilities for parsing command parameters
	template < >
	STARFIREUTILITIES_API int GetParams( const TCHAR *&Cmd, int &output );
	template < >
	STARFIREUTILITIES_API int GetParams( const TCHAR *&Cmd, float &output );
	template < >
	STARFIREUTILITIES_API int GetParams( const TCHAR *&Cmd, FString &output );
	template < >
	STARFIREUTILITIES_API int GetParams( const TCHAR *&Cmd, FName &output );
	template < >
	STARFIREUTILITIES_API int GetParams( const TCHAR *&Cmd, bool &output );
}

// Utility class to aid in the creation of console commands and the parsing of console command input strings
struct STARFIREUTILITIES_API FExecSF : FSelfRegisteringExec
{
	// Delegate type for the console command functions
	DECLARE_DELEGATE_ThreeParams( FExecDelegate, const UWorld*, const TCHAR*, FOutputDevice& );

	// Self Registering Exec API
	bool Exec( UWorld *pWorld, const TCHAR *Cmd, FOutputDevice &Ar ) override;

	// Add a console command that can be dispatched by this instance
	void AddExec( const TCHAR *Cmd, const TCHAR *Help, const FExecDelegate &CmdDelegate );

private:
	// The console commands registered with this utility
	TMap< FString, FExecDelegate > RegisteredExecs;
};

#if CPP
#define SF_EXEC_HPP
#include "../../Private/Misc/ExecSF.hpp"
#undef SF_EXEC_HPP
#endif