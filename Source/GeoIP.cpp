/**
 * @file
 * @brief Implementation of GeoIP class.
 */

#include "GeoIP.hpp"
#include "Log.hpp"
#include "conntop_config.h"

#ifdef CONNTOP_USE_OWN_LIBMAXMINDDB
#include "maxminddb.h"
#else
#include <maxminddb.h>
#endif

namespace ctp
{
	static Country ParseCountryData( MMDB_lookup_result_s & result, const IAddress & address )
	{
		if ( ! result.found_entry )
		{
			return Country();
		}

		MMDB_entry_data_s countryEntry;
		int status = MMDB_get_value( &result.entry, &countryEntry, "country", "iso_code", nullptr );
		if ( status != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] Country data of address %s parse error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( status )
			);
			return Country();
		}

		if ( ! countryEntry.has_data || countryEntry.type != MMDB_DATA_TYPE_UTF8_STRING )
		{
			gLog->error( "[GeoIP] Invalid country data of address %s", address.toString().c_str() );
			return Country();
		}

		return Country::ParseCodeString( KString( countryEntry.utf8_string, countryEntry.data_size ) );
	}

	static ASN ParseASNData( MMDB_lookup_result_s & result, const IAddress & address )
	{
		if ( ! result.found_entry )
		{
			return ASN();
		}

		MMDB_entry_data_s numEntry;
		int numStatus = MMDB_get_value( &result.entry, &numEntry, "autonomous_system_number", nullptr );
		if ( numStatus != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] AS number data of address %s parse error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( numStatus )
			);
			return ASN();
		}

		if ( ! numEntry.has_data || numEntry.type != MMDB_DATA_TYPE_UINT32 )
		{
			gLog->error( "[GeoIP] Invalid AS number data of address %s", address.toString().c_str() );
			return ASN();
		}

		MMDB_entry_data_s orgEntry;
		int orgStatus = MMDB_get_value( &result.entry, &orgEntry, "autonomous_system_organization", nullptr );
		if ( orgStatus != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] AS organization name data of address %s parse error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( orgStatus )
			);
			return ASN();
		}

		if ( ! orgEntry.has_data || orgEntry.type != MMDB_DATA_TYPE_UTF8_STRING )
		{
			gLog->error( "[GeoIP] Invalid AS organization name data of address %s", address.toString().c_str() );
			return ASN();
		}

		return ASN( numEntry.uint32, std::string( orgEntry.utf8_string, orgEntry.data_size ) );
	}

	class GeoIP::Impl
	{
		MMDB_s m_dbCountry;
		MMDB_s m_dbASN;
		bool m_hasDB_Country;
		bool m_hasDB_ASN;

		void loadDB_Country()
		{
			GeoIP::DBSearchPaths searchPaths;
			for ( std::string path = searchPaths.getNext(); ! path.empty(); path = searchPaths.getNext() )
			{
				path += GeoIP::DB_COUNTRY_FILENAME;
				int status = MMDB_open( path.c_str(), MMDB_MODE_MMAP, &m_dbCountry );
				if ( status == MMDB_SUCCESS )
				{
					m_hasDB_Country = true;
					gLog->info( "[GeoIP] Country database '%s' loaded", path.c_str() );
					return;
				}
				else
				{
					gLog->debug( "[GeoIP] Cannot load '%s': %s", path.c_str(), MMDB_strerror( status ) );
				}
			}
			m_hasDB_Country = false;
			gLog->warning( "[GeoIP] Unable to load any country database" );
		}

		void loadDB_ASN()
		{
			GeoIP::DBSearchPaths searchPaths;
			for ( std::string path = searchPaths.getNext(); ! path.empty(); path = searchPaths.getNext() )
			{
				path += GeoIP::DB_ASN_FILENAME;
				int status = MMDB_open( path.c_str(), MMDB_MODE_MMAP, &m_dbASN );
				if ( status == MMDB_SUCCESS )
				{
					m_hasDB_ASN = true;
					gLog->info( "[GeoIP] ASN database '%s' loaded", path.c_str() );
					return;
				}
				else
				{
					gLog->debug( "[GeoIP] Cannot load '%s': %s", path.c_str(), MMDB_strerror( status ) );
				}
			}
			m_hasDB_ASN = false;
			gLog->warning( "[GeoIP] Unable to load any ASN database" );
		}

	public:
		Impl()
		: m_dbCountry(),
		  m_dbASN(),
		  m_hasDB_Country(),
		  m_hasDB_ASN()
		{
			loadDB_Country();
			loadDB_ASN();
		}

		~Impl()
		{
			if ( m_hasDB_Country )
			{
				MMDB_close( &m_dbCountry );
				gLog->info( "[GeoIP] Country database closed" );
			}

			if ( m_hasDB_ASN )
			{
				MMDB_close( &m_dbASN );
				gLog->info( "[GeoIP] ASN database closed" );
			}
		}

		bool hasDB_Country() const
		{
			return m_hasDB_Country;
		}

		bool hasDB_ASN() const
		{
			return m_hasDB_ASN;
		}

		const char *getDBFileName_Country() const
		{
			return (m_hasDB_Country) ? m_dbCountry.filename : nullptr;
		}

		const char *getDBFileName_ASN() const
		{
			return (m_hasDB_ASN) ? m_dbASN.filename : nullptr;
		}

		MMDB_s *getDB_Country()
		{
			return (m_hasDB_Country) ? &m_dbCountry : nullptr;
		}

		MMDB_s *getDB_ASN()
		{
			return (m_hasDB_ASN) ? &m_dbASN : nullptr;
		}
	};

	GeoIP::GeoIP()
	: m_impl(std::make_unique<Impl>())
	{
	}

	GeoIP::~GeoIP()
	{
	}

	bool GeoIP::hasDB_Country() const
	{
		return m_impl->hasDB_Country();
	}

	bool GeoIP::hasDB_ASN() const
	{
		return m_impl->hasDB_ASN();
	}

	KString GeoIP::getDBFileName_Country() const
	{
		return m_impl->getDBFileName_Country();
	}

	KString GeoIP::getDBFileName_ASN() const
	{
		return m_impl->getDBFileName_ASN();
	}

	Country GeoIP::queryCountry( const AddressIP4 & address )
	{
		MMDB_s *pDatabase = m_impl->getDB_Country();
		if ( ! pDatabase )
		{
			// country database is not available
			return Country();
		}

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		address.copyRawTo( &addr.sin_addr );

		const sockaddr *pAddr = reinterpret_cast<const sockaddr*>( &addr );

		int status;
		MMDB_lookup_result_s result = MMDB_lookup_sockaddr( pDatabase, pAddr, &status );
		if ( status != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] Country data of address %s query error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( status )
			);
			return Country();
		}

		return ParseCountryData( result, address );
	}

	Country GeoIP::queryCountry( const AddressIP6 & address )
	{
		MMDB_s *pDatabase = m_impl->getDB_Country();
		if ( ! pDatabase )
		{
			// country database is not available
			return Country();
		}

		sockaddr_in6 addr{};
		addr.sin6_family = AF_INET6;
		address.copyRawTo( &addr.sin6_addr );

		const sockaddr *pAddr = reinterpret_cast<const sockaddr*>( &addr );

		int status;
		MMDB_lookup_result_s result = MMDB_lookup_sockaddr( pDatabase, pAddr, &status );
		if ( status != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] Country data of address %s query error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( status )
			);
			return Country();
		}

		return ParseCountryData( result, address );
	}

	ASN GeoIP::queryASN( const AddressIP4 & address )
	{
		MMDB_s *pDatabase = m_impl->getDB_ASN();
		if ( ! pDatabase )
		{
			// ASN database is not available
			return ASN();
		}

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		address.copyRawTo( &addr.sin_addr );

		const sockaddr *pAddr = reinterpret_cast<const sockaddr*>( &addr );

		int status;
		MMDB_lookup_result_s result = MMDB_lookup_sockaddr( pDatabase, pAddr, &status );
		if ( status != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] ASN data of address %s query error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( status )
			);
			return ASN();
		}

		return ParseASNData( result, address );
	}

	ASN GeoIP::queryASN( const AddressIP6 & address )
	{
		MMDB_s *pDatabase = m_impl->getDB_ASN();
		if ( ! pDatabase )
		{
			// ASN database is not available
			return ASN();
		}

		sockaddr_in6 addr{};
		addr.sin6_family = AF_INET6;
		address.copyRawTo( &addr.sin6_addr );

		const sockaddr *pAddr = reinterpret_cast<const sockaddr*>( &addr );

		int status;
		MMDB_lookup_result_s result = MMDB_lookup_sockaddr( pDatabase, pAddr, &status );
		if ( status != MMDB_SUCCESS )
		{
			gLog->error( "[GeoIP] ASN data of address %s query error: %s",
			  address.toString().c_str(),
			  MMDB_strerror( status )
			);
			return ASN();
		}

		return ParseASNData( result, address );
	}
}
