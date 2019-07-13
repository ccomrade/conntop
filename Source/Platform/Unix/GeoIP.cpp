/**
 * @file
 * @brief Implementation of platform-specific stuff from GeoIP class for Unix platform.
 */

#include <cstdlib>  // std::getenv

#include "GeoIP.hpp"
#include "conntop_config.h"

namespace ctp
{
	const KString GeoIP::DB_COUNTRY_FILENAME = "GeoLite2-Country.mmdb";
	const KString GeoIP::DB_ASN_FILENAME = "GeoLite2-ASN.mmdb";

	enum
	{
		DB_PATH_HOME,
		DB_PATH_SYSTEM,
		DB_PATH_SYSTEM_VAR
	};

	GeoIP::DBSearchPaths::DBSearchPaths()
	: m_currentPath(0)
	{
	}

	std::string GeoIP::DBSearchPaths::getNext()
	{
		std::string path;
		switch ( m_currentPath )
		{
			case DB_PATH_HOME:
			{
				const char *homeDir = std::getenv( "HOME" );
				if ( homeDir == nullptr )
				{
					// skip current path
					m_currentPath++;
					return getNext();
				}
				path = homeDir;
				path += "/.local/share/GeoIP/";
				break;
			}
			case DB_PATH_SYSTEM:
			{
				path = CONNTOP_INSTALL_PREFIX "/share/GeoIP/";
				break;
			}
			case DB_PATH_SYSTEM_VAR:
			{
				path = "/var/lib/GeoIP/";
				break;
			}
			default:
			{
				// no more paths, so return empty string
				return path;
			}
		}

		// advance to the next path
		m_currentPath++;

		return path;
	}
}
