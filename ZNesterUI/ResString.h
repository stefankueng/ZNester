#pragma once
#include <string>

/**
 * Loads a string from the application resources.
 */

inline std::wstring LoadResourceWString( HINSTANCE hInstance, UINT uID )
{
	const wchar_t* p   = nullptr;
	int			   len = ::LoadStringW( hInstance, uID, reinterpret_cast<LPWSTR>( &p ), 0 );
	return ( len > 0 ) ? std::wstring( p, static_cast<size_t>( len ) ) : std::wstring();
}

class ResString
{
public:
	// hInst is necessary to support multiple languages with resource dlls
	inline ResString( HINSTANCE hInst, int resId )
		: m_str( LoadResourceWString( hInst, resId ) )
	{
	}
	// Handy for when used with printf etc and to avoid explicit casts.
	inline const wchar_t* c_str() const
	{
		return m_str.c_str();
	}
	inline operator const wchar_t* const() const
	{
		return m_str.c_str();
	}
	inline operator const std::wstring&() const
	{
		return m_str;
	}

private:
	const std::wstring m_str;
};
