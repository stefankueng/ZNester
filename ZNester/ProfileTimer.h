#pragma once
#include <format>
#include <string>

class ProfileTimer
{
public:
	ProfileTimer( LPCWSTR text )
		: info( text )
	{
		QueryPerformanceCounter( &startTime );
	}

	~ProfileTimer()
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter( &endTime );
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency( &frequency );
		LARGE_INTEGER milliseconds;
		milliseconds.QuadPart  = endTime.QuadPart - startTime.QuadPart;
		milliseconds.QuadPart *= 1000;
		milliseconds.QuadPart /= frequency.QuadPart;
		auto output            = std::format( L"{0} : {1} ms\n", info.c_str(), milliseconds.QuadPart );
		OutputDebugString( output.c_str() );
	}

private:
	LARGE_INTEGER startTime;
	std::wstring  info;
};
