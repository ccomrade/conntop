/**
 * @file
 * @brief ASN class.
 */

#pragma once

#include <string>

#include "Types.hpp"

namespace ctp
{
	/**
	 * @brief Autonomous system number.
	 */
	class ASN
	{
		//! RFC 6793 defines ASN as four-octet (32-bit) entity.
		uint32_t m_number;
		//! ASN as string, e.g. "AS12345".
		std::string m_string;
		//! AS organization name.
		std::string m_orgName;

	public:
		ASN()
		: m_number(0),  // zero doesn't represent any valid AS
		  m_string(),
		  m_orgName()
		{
		}

		ASN( uint32_t number, std::string && orgName )
		: m_number(number),
		  m_string(),
		  m_orgName(std::move( orgName ))
		{
			m_string = "AS";
			m_string += std::to_string( m_number );
		}

		bool isEmpty() const
		{
			return m_number == 0;
		}

		uint32_t getNumber() const
		{
			return m_number;
		}

		const std::string & getString() const
		{
			return m_string;
		}

		const std::string & getOrgName() const
		{
			return m_orgName;
		}
	};

	inline bool operator==( const ASN & a, const ASN & b )
	{
		return a.getNumber() == b.getNumber();
	}

	inline bool operator!=( const ASN & a, const ASN & b )
	{
		return a.getNumber() != b.getNumber();
	}

	inline bool operator<( const ASN & a, const ASN & b )
	{
		return a.getNumber() < b.getNumber();
	}

	inline bool operator>( const ASN & a, const ASN & b )
	{
		return a.getNumber() > b.getNumber();
	}

	inline bool operator<=( const ASN & a, const ASN & b )
	{
		return a.getNumber() <= b.getNumber();
	}

	inline bool operator>=( const ASN & a, const ASN & b )
	{
		return a.getNumber() >= b.getNumber();
	}
}
