
// ReSharper disable once CppMissingIncludeGuard
#ifndef SF_EXEC_HPP
	#error You shouldn't be including this file directly
#endif

template < typename First_t, typename... Args_t >
int ExecSF_Params::GetParams( const TCHAR *&Cmd, First_t &First, Args_t&... Args )
{
	int count = GetParams< First_t >( Cmd, First );

	if (count > 0)
		count += GetParams( Cmd, Args... );

	return count;
}