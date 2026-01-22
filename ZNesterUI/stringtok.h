#pragma once
#include <string>

// append = true as the default: a default value should never lose data!
template <typename Container>
void stringtok( Container& container, const std::wstring& in, bool trim, const wchar_t* const delimiters = L"|",
				bool append = true )
{
	const std::wstring::size_type len = in.length();
	std::wstring::size_type       i   = 0;
	if ( !append )
		container.clear();

	while ( i < len )
	{
		if ( trim )
		{
			// eat leading whitespace
			i = in.find_first_not_of( delimiters, i );
			if ( i == std::wstring::npos )
				return;  // nothing left but white space
		}

		// find the end of the token
		std::wstring::size_type j = in.find_first_of( delimiters, i );

		// push token
		if ( j == std::wstring::npos )
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::wstring> )
			{
				container.push_back( in.substr( i ) );
			}
			else
			{
				container.push_back( static_cast<typename Container::value_type>( _wtoi64( in.substr( i ).c_str() ) ) );
			}
			return;
		}
		else
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::wstring> )
			{
				container.push_back( in.substr( i, j - i ) );
			}
			else
			{
				container.push_back(
					static_cast<typename Container::value_type>( _wtoi64( in.substr( i, j - i ).c_str() ) ) );
			}
		}

		// set up for next loop
		i = j + 1;
	}
}

template <typename Container>
void stringtokset( Container& container, const std::wstring& in, bool trim, const wchar_t* const delimiters = L"|",
				   bool append = false )
{
	const std::wstring::size_type len = in.length();
	std::wstring::size_type       i   = 0;
	if ( !append )
		container.clear();

	while ( i < len )
	{
		if ( trim )
		{
			// eat leading whitespace
			i = in.find_first_not_of( delimiters, i );
			if ( i == std::wstring::npos )
				return;  // nothing left but white space
		}

		// find the end of the token
		std::wstring::size_type j = in.find_first_of( delimiters, i );

		// push token
		if ( j == std::wstring::npos )
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::wstring> )
			{
				container.insert( in.substr( i ) );
			}
			else
			{
				container.insert( static_cast<typename Container::value_type>( _wtoi64( in.substr( i ).c_str() ) ) );
			}
			return;
		}
		else
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::wstring> )
			{
				container.insert( in.substr( i, j - i ) );
			}
			else
			{
				container.insert(
					static_cast<typename Container::value_type>( _wtoi64( in.substr( i, j - i ).c_str() ) ) );
			}
		}

		// set up for next loop
		i = j + 1;
	}
}

// append = true as the default: a default value should never lose data!
template <typename Container>
void stringtok( Container& container, const std::string& in, bool trim, const char* const delimiters = "|",
				bool append = true )
{
	const std::string::size_type len = in.length();
	std::string::size_type       i   = 0;
	if ( !append )
		container.clear();

	while ( i < len )
	{
		if ( trim )
		{
			// eat leading whitespace
			i = in.find_first_not_of( delimiters, i );
			if ( i == std::string::npos )
				return;  // nothing left but white space
		}

		// find the end of the token
		std::string::size_type j = in.find_first_of( delimiters, i );

		// push token
		if ( j == std::string::npos )
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::string> )
			{
				container.push_back( in.substr( i ) );
			}
			else
			{
				container.push_back( static_cast<typename Container::value_type>( _atoi64( in.substr( i ).c_str() ) ) );
			}
			return;
		}
		else
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::string> )
			{
				container.push_back( in.substr( i, j - i ) );
			}
			else
			{
				container.push_back(
					static_cast<typename Container::value_type>( _atoi64( in.substr( i, j - i ).c_str() ) ) );
			}
		}

		// set up for next loop
		i = j + 1;
	}
}

template <typename Container>
void stringtokset( Container& container, const std::string& in, bool trim, const char* const delimiters = "|",
				   bool append = false )
{
	const std::string::size_type len = in.length();
	std::string::size_type       i   = 0;
	if ( !append )
		container.clear();

	while ( i < len )
	{
		if ( trim )
		{
			// eat leading whitespace
			i = in.find_first_not_of( delimiters, i );
			if ( i == std::string::npos )
				return;  // nothing left but white space
		}

		// find the end of the token
		std::string::size_type j = in.find_first_of( delimiters, i );

		// push token
		if ( j == std::string::npos )
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::string> )
			{
				container.insert( in.substr( i ) );
			}
			else
			{
				container.insert( static_cast<typename Container::value_type>( _atoi64( in.substr( i ).c_str() ) ) );
			}
			return;
		}
		else
		{
			if constexpr ( std::is_same_v<typename Container::value_type, std::string> )
			{
				container.insert( in.substr( i, j - i ) );
			}
			else
			{
				container.insert(
					static_cast<typename Container::value_type>( _atoi64( in.substr( i, j - i ).c_str() ) ) );
			}
		}

		// set up for next loop
		i = j + 1;
	}
}
