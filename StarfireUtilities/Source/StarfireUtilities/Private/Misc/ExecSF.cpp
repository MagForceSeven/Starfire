
#include "Misc/ExecSF.h"

// Core
#include "HAL/IConsoleManager.h"
#include "Misc/Parse.h"

bool FExecSF::Exec( UWorld *pWorld, const TCHAR *Cmd, FOutputDevice &Ar )
{
	const FString ExecCmd = FParse::Token( Cmd, true );

	FExecDelegate *CmdDelegate = RegisteredExecs.Find( ExecCmd );
	if (CmdDelegate != nullptr)
	{
		CmdDelegate->Execute( pWorld, Cmd, Ar );
		return true;
	}

	return false;
}

void FExecSF::AddExec( const TCHAR *Cmd, const TCHAR *Help, const FExecDelegate &CmdDelegate )
{
	RegisteredExecs.Add( Cmd, CmdDelegate );
	IConsoleManager::Get( ).RegisterConsoleCommand( Cmd, Help, ECVF_Default );
}

template < >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, int &output )
{
	const auto Param = FParse::Token( Cmd, true );
	if (Param.IsEmpty( ) || !Param.IsNumeric( ))
		return 0;

	output = FCString::Atoi( *Param );
	return 1;
}

template < >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, float &output )
{
	const auto Param = FParse::Token( Cmd, true );
	if (Param.IsEmpty( ) || !Param.IsNumeric())
		return 0;

	output = FCString::Atof( *Param );
	return 1;
}

template < >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, FString &output )
{
	output = FParse::Token( Cmd, true );
	if (output.IsEmpty( ))
		return 0;

	return 1;
}

template < >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, FName &output )
{
	const auto Param = FParse::Token( Cmd, true );
	if (Param.IsEmpty( ))
		return 0;

	output = *Param;
	return 1;
}

template < >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, bool &output )
{
	auto Param = FParse::Token( Cmd, true );
	if (Param.IsEmpty( ))
		return 0;

	Param.RemoveSpacesInline( );

	const FString TrueStrings[ ] = { "1", "t", "true" };
	const FString FalseStrings[ ] = { "0", "f", "false" };

	for (const auto &Test : TrueStrings)
	{
		if (Param.Compare( Test, ESearchCase::IgnoreCase ))
		{
			output = true;
			return 1;
		}
	}

	for (const auto &Test : FalseStrings)
	{
		if (Param.Compare( Test, ESearchCase::IgnoreCase ))
		{
			output = false;
			return 1;
		}
	}


	return 0;
}

using namespace ExecSF_Params;
struct FCoreExecTester : public FExecSF
{
	FCoreExecTester( )
	{
		AddExec( TEXT( "Starfire.Execs.ParamsTest" ), TEXT( "" ), FExecDelegate::CreateStatic( &FCoreExecTester::ParamsTest ) );
	}

	// ReSharper disable once CppParameterValueIsReassigned
	static void ParamsTest( const UWorld *World, const TCHAR *Cmd, FOutputDevice &Ar )
	{
		FString StringTest[ 4 ];
		int IntTest[ 4 ];
		float FloatTest[ 4 ];
		FName NameTest[ 4 ];

		Cmd = TEXT( "Alpha" );
		// ReSharper disable once CppEntityAssignedButNoRead
		int count = GetParams( Cmd, StringTest[ 0 ], StringTest[ 1 ], StringTest[ 2 ], StringTest[ 3 ] );
		Cmd = TEXT( "Bravo Charlie" );
		// ReSharper disable CppAssignedValueIsNeverUsed
		count = GetParams( Cmd, StringTest[ 0 ], StringTest[ 1 ], StringTest[ 2 ], StringTest[ 3 ] );
		Cmd = TEXT( "Delta Echo Foxtrot" );
		count = GetParams( Cmd, StringTest[ 0 ], StringTest[ 1 ], StringTest[ 2 ], StringTest[ 3 ] );
		Cmd = TEXT( "Golf Hotel Indigo Juliet" );
		count = GetParams( Cmd, StringTest[ 0 ], StringTest[ 1 ], StringTest[ 2 ], StringTest[ 3 ] );

		Cmd = TEXT( "Alpha" );
		count = GetParams( Cmd, NameTest[ 0 ], NameTest[ 1 ], NameTest[ 2 ], NameTest[ 3 ] );
		Cmd = TEXT( "Bravo Charlie" );
		count = GetParams( Cmd, NameTest[ 0 ], NameTest[ 1 ], NameTest[ 2 ], NameTest[ 3 ] );
		Cmd = TEXT( "Delta Echo Foxtrot" );
		count = GetParams( Cmd, NameTest[ 0 ], NameTest[ 1 ], NameTest[ 2 ], NameTest[ 3 ] );
		Cmd = TEXT( "Golf Hotel Indigo Juliet" );
		count = GetParams( Cmd, NameTest[ 0 ], NameTest[ 1 ], NameTest[ 2 ], NameTest[ 3 ] );

		Cmd = TEXT( "1.2" );
		count = GetParams( Cmd, FloatTest[ 0 ], FloatTest[ 1 ], FloatTest[ 2 ], FloatTest[ 3 ] );
		Cmd = TEXT( "2.2 3.14159" );
		count = GetParams( Cmd, FloatTest[ 0 ], FloatTest[ 1 ], FloatTest[ 2 ], FloatTest[ 3 ] );
		Cmd = TEXT( "4 5.0 6.12" );
		count = GetParams( Cmd, FloatTest[ 0 ], FloatTest[ 1 ], FloatTest[ 2 ], FloatTest[ 3 ] );
		Cmd = TEXT( "7.14 8.16 9.18 10.2" );
		count = GetParams( Cmd, FloatTest[ 0 ], FloatTest[ 1 ], FloatTest[ 2 ], FloatTest[ 3 ] );

		Cmd = TEXT( "1.2" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ], IntTest[ 2 ], IntTest[ 3 ] );
		Cmd = TEXT( "2.2 3.14159" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ], IntTest[ 2 ], IntTest[ 3 ] );
		Cmd = TEXT( "4 5.0 6.12" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ], IntTest[ 2 ], IntTest[ 3 ] );
		Cmd = TEXT( "7.14 8.16 9.18 10.2" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ], IntTest[ 2 ], IntTest[ 3 ] );

		Cmd = TEXT( "" );
		count = GetParams( Cmd, IntTest[ 0 ] );
		Cmd = TEXT( "Alpha" );
		count = GetParams( Cmd, IntTest[ 0 ] );
		Cmd = TEXT( "Alpha Bravo" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ] );
		Cmd = TEXT( "3.2 Bravo" );
		count = GetParams( Cmd, IntTest[ 0 ], IntTest[ 1 ] );
		Cmd = TEXT( "3.2 Bravo" );
		count = GetParams( Cmd, IntTest[ 0 ], StringTest[ 1 ] );
		Cmd = TEXT( "3.2 Bravo" );
		count = GetParams( Cmd, StringTest[ 0 ], IntTest[ 1 ] );
		// ReSharper restore CppAssignedValueIsNeverUsed
	}

} GCoreExecTester;