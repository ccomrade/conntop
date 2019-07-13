/**
 * @file
 * @brief KString class.
 */

#pragma once

#include <string>
#include <ostream>

#include "Types.hpp"
#include "Compiler.hpp"

namespace ctp
{
	/**
	 * @brief Compile-time C string wrapper similar to std::string_view from C++17.
	 */
	class KString
	{
		const char *m_string;
		size_t m_length;

		static constexpr size_t ComputeLength( const char *string ) noexcept
		{
		#ifdef COMPILER_CONSTEXPR_STRLEN
			return COMPILER_CONSTEXPR_STRLEN( string );
		#else
			size_t i = 0;
			while ( string[i] != '\0' )
			{
				i++;
			}
			return i;
		#endif
		}

	public:
		constexpr KString() noexcept
		: m_string(""),
		  m_length(0)
		{
		}

		constexpr KString( const char *string ) noexcept
		: m_string((string == nullptr) ? "" : string),
		  m_length(ComputeLength(m_string))
		{
		}

		constexpr KString( const char *string, size_t length ) noexcept
		: m_string(string),
		  m_length(length)
		{
		}

		KString( const std::string & string ) noexcept
		: m_string(string.c_str()),
		  m_length(string.length())
		{
		}

		constexpr KString( const KString & ) noexcept = default;

		constexpr KString & operator=( const KString & ) noexcept = default;

		constexpr const char *c_str() const noexcept
		{
			return m_string;
		}

		constexpr size_t length() const noexcept
		{
			return m_length;
		}

		constexpr size_t size() const noexcept
		{
			return m_length;
		}

		constexpr bool empty() const noexcept
		{
			return m_length == 0;
		}

		constexpr char operator[]( size_t pos ) const noexcept
		{
			return m_string[pos];
		}

		operator std::string() const
		{
			return std::string( m_string, m_length );
		}

		constexpr int compare( const KString & other ) const noexcept
		{
			const size_t length = (m_length <= other.m_length) ? m_length : other.m_length;

			for ( size_t i = 0; i < length; i++ )
			{
				if ( m_string[i] != other.m_string[i] )
				{
					return (m_string[i] < other.m_string[i]) ? -1 : 1;
				}
			}

			if ( m_length != other.m_length )
			{
				return (m_length < other.m_length) ? -1 : 1;
			}

			return 0;
		}
	};

	inline constexpr bool operator==( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) == 0;
	}

	inline constexpr bool operator!=( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) != 0;
	}

	inline constexpr bool operator<( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) < 0;
	}

	inline constexpr bool operator>( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) > 0;
	}

	inline constexpr bool operator<=( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) <= 0;
	}

	inline constexpr bool operator>=( const KString & a, const KString & b ) noexcept
	{
		return a.compare( b ) >= 0;
	}

	inline std::string & operator+=( std::string & self, const KString & string )
	{
		self.append( string.c_str(), string.length() );
		return self;
	}

	inline std::ostream & operator<<( std::ostream & stream, const KString & string )
	{
		stream << string.c_str();
		return stream;
	}
}
