/**
 * @file
 * @brief GeoIP class.
 */

#pragma once

#include <string>
#include <memory>

#include "KString.hpp"
#include "Address.hpp"
#include "Country.hpp"
#include "ASN.hpp"

namespace ctp
{
	class GeoIP
	{
		static const KString DB_COUNTRY_FILENAME;
		static const KString DB_ASN_FILENAME;

		class DBSearchPaths
		{
			int m_currentPath;

		public:
			DBSearchPaths();

			std::string getNext();
		};

		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		GeoIP();
		~GeoIP();

		bool hasDB_Country() const;
		bool hasDB_ASN() const;

		KString getDBFileName_Country() const;
		KString getDBFileName_ASN() const;

		Country queryCountry(const AddressIP4 & address);
		Country queryCountry(const AddressIP6 & address);

		Country queryCountry(const IAddress & address)
		{
			switch (address.getType())
			{
				case EAddressType::IP4:
				{
					return queryCountry(static_cast<const AddressIP4&>(address));
				}
				case EAddressType::IP6:
				{
					return queryCountry(static_cast<const AddressIP6&>(address));
				}
			}

			return Country();
		}

		ASN queryASN(const AddressIP4 & address);
		ASN queryASN(const AddressIP6 & address);

		ASN queryASN(const IAddress & address)
		{
			switch (address.getType())
			{
				case EAddressType::IP4:
				{
					return queryASN(static_cast<const AddressIP4&>(address));
				}
				case EAddressType::IP6:
				{
					return queryASN(static_cast<const AddressIP6&>(address));
				}
			}

			return ASN();
		}
	};
}
