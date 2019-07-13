/**
 * @file
 * @brief Implementation of platform-specific functions from network address classes for Unix platform.
 */

#include <arpa/inet.h>
#include <stdexcept>  // std::invalid_argument

#include "Address.hpp"

namespace ctp
{
	std::string AddressIP4::toString() const
	{
		char buffer[INET_ADDRSTRLEN];
		if ( inet_ntop( AF_INET, &m_address, buffer, sizeof buffer ) == nullptr )
		{
			return std::string();
		}

		return std::string( buffer );
	}

	std::string AddressIP6::toString() const
	{
		char buffer[INET6_ADDRSTRLEN];
		if ( inet_ntop( AF_INET6, m_address, buffer, sizeof buffer ) == nullptr )
		{
			return std::string();
		}

		return std::string( buffer );
	}

	AddressIP4 AddressIP4::CreateFromString( const KString & string )
	{
		in_addr result;
		if ( inet_pton( AF_INET, string.c_str(), &result ) <= 0 )
		{
			throw std::invalid_argument( "Invalid IPv4 address string" );
		}

		return AddressIP4( result.s_addr );
	}

	AddressIP6 AddressIP6::CreateFromString( const KString & string )
	{
		in6_addr result;
		if ( inet_pton( AF_INET6, string.c_str(), &result ) <= 0 )
		{
			throw std::invalid_argument( "Invalid IPv6 address string" );
		}

		return AddressIP6( result.s6_addr32 );
	}
}
