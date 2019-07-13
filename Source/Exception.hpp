/**
 * @file
 * @brief Exception class.
 */

#pragma once

#include <string>
#include <exception>

#include "Log.hpp"

namespace ctp
{
	class Exception : public std::exception
	{
		std::string m_what;
		const char *m_origin;
		bool m_wasLogAvailable;

	public:
		Exception( std::string what, const char *origin = nullptr, bool log = true )
		: m_what(std::move( what )),
		  m_origin(origin),
		  m_wasLogAvailable()
		{
			if ( gLog && gLog->getVerbosity() != Log::VERBOSITY_DISABLED )
			{
				m_wasLogAvailable = true;

				if ( log )
				{
					if ( m_origin )
					{
						gLog->error( "[%s] %s", m_origin, m_what.c_str() );
					}
					else
					{
						gLog->error( "%s", m_what.c_str() );
					}
				}
			}
			else
			{
				m_wasLogAvailable = false;
			}
		}

		const char *what() const noexcept override
		{
			return m_what.c_str();
		}

		bool wasLogAvailable() const
		{
			return m_wasLogAvailable;
		}

		bool hasOrigin() const
		{
			return (m_origin);
		}

		const char *getOrigin() const
		{
			return m_origin;
		}

		const std::string & getString() const
		{
			return m_what;
		}
	};
}
