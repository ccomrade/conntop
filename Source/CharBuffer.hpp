/**
 * @file
 * @brief CharBuffer class.
 */

#pragma once

#include <cstdio>  // std::vsnprintf
#include <cstdarg>
#include <cstring>  // std::memcpy
#include <string>

#include "Types.hpp"
#include "KString.hpp"
#include "Compiler.hpp"

namespace ctp
{
	template<size_t Size>
	class CharBuffer
	{
		static_assert( Size > 1, "Size of CharBuffer must be at least 2" );

		char m_buffer[Size];
		size_t m_pos;

	public:
		CharBuffer()
		{
			m_pos = 0;
			m_buffer[m_pos] = '\0';
		}

		bool isEmpty() const
		{
			return m_pos == 0;
		}

		bool isFull() const
		{
			return getAvailableLength() == 0;
		}

		size_t getDataLength() const
		{
			return m_pos;
		}

		size_t getAvailableLength() const
		{
			return Size - m_pos - 1;
		}

		size_t getCapacity() const
		{
			return Size;
		}

		KString get() const
		{
			return KString( m_buffer, m_pos );
		}

		std::string toString() const
		{
			return std::string( m_buffer, m_pos );
		}

		char operator[]( size_t index ) const
		{
			return m_buffer[index];
		}

		CharBuffer<Size> & operator+=( char c )
		{
			append( c );
			return *this;
		}

		CharBuffer<Size> & operator+=( const KString & string )
		{
			append( string );
			return *this;
		}

		size_t append( char c )
		{
			if ( getAvailableLength() > 0 )
			{
				m_buffer[m_pos] = c;
				m_pos++;
				m_buffer[m_pos] = '\0';

				return 1;
			}
			else
			{
				return 0;
			}
		}

		size_t append( const KString & string )
		{
			size_t length = string.length();
			if ( length == 0 )
			{
				return 0;
			}

			size_t availableLength = getAvailableLength();
			if ( length > availableLength )
			{
				length = availableLength;
			}

			std::memcpy( m_buffer+m_pos, string.c_str(), length );
			m_pos += length;
			m_buffer[m_pos] = '\0';

			return length;
		}

		int append_f( const char *format, ... ) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start( args, format );
			int status = std::vsnprintf( m_buffer+m_pos, Size-m_pos, format, args );
			va_end( args );

			if ( status > 0 )
			{
				size_t newDataLength = status;
				size_t availableLength = getAvailableLength();
				m_pos += (newDataLength > availableLength) ? availableLength : newDataLength;
			}

			return status;
		}

		int append_vf( const char *format, va_list args )
		{
			int status = std::vsnprintf( m_buffer+m_pos, Size-m_pos, format, args );

			if ( status > 0 )
			{
				size_t newDataLength = status;
				size_t availableLength = getAvailableLength();
				m_pos += (newDataLength > availableLength) ? availableLength : newDataLength;
			}

			return status;
		}
	};
}
