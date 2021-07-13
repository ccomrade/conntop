/**
 * @file
 * @brief Network address classes.
 */

#pragma once

#include <cstring>  // std::memcpy
#include <string>
#include <vector>
#include <memory>

#include "Types.hpp"
#include "KString.hpp"
#include "Hash.hpp"
#include "conntop_config.h"

#ifndef CONNTOP_DEDICATED
#include "ASN.hpp"
#include "Country.hpp"
#include "WhoisData.hpp"
#endif

namespace ctp
{
	/**
	 * @brief Network address type (L3 protocol).
	 */
	enum struct EAddressType
	{
		IP4,  //!< IPv4 address
		IP6   //!< IPv6 address
	};

	/**
	 * @brief Network address base class.
	 */
	class IAddress
	{
	protected:
		/**
		 * @brief Protected default constructor.
		 */
		IAddress() = default;

	public:
		/**
		 * @brief Virtual destructor.
		 */
		virtual ~IAddress() = default;

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		virtual EAddressType getType() const = 0;

		/**
		 * @brief Returns name of address type.
		 * @return Address type name.
		 */
		virtual KString getTypeName() const = 0;

		/**
		 * @brief Converts address to string.
		 * @return Address string.
		 */
		virtual std::string toString() const = 0;

		/**
		 * @brief Copies raw address to some buffer.
		 * @param buffer The buffer.
		 */
		virtual void copyRawTo(void *buffer) const = 0;
	};

	/**
	 * @brief IPv4 address.
	 */
	class AddressIP4 : public IAddress
	{
		//! Entire 32-bit IPv4 address in network byte order (big-endian).
		uint32_t m_address;

	public:
		/**
		 * @brief Constructor.
		 * @param address Raw IPv4 address in network byte order.
		 */
		AddressIP4(uint32_t address)
		: m_address(address)
		{
		}

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		EAddressType getType() const override
		{
			return EAddressType::IP4;
		}

		/**
		 * @brief Returns name of address type.
		 * @return Address type name.
		 */
		KString getTypeName() const override
		{
			return "IPv4";
		}

		/**
		 * @brief Converts IPv4 address to string.
		 * No lookups are performed. Implementation is platform-specific.
		 * @return IPv4 address string.
		 */
		std::string toString() const override;

		/**
		 * @brief Copies raw IPv4 address to some buffer.
		 * Size of the buffer must be at least 4 bytes.
		 * @param buffer The buffer.
		 */
		void copyRawTo(void *buffer) const override
		{
			std::memcpy(buffer, &m_address, 4);
		}

		/**
		 * @brief Returns raw IPv4 address.
		 * @return Raw IPv4 address in network byte order.
		 */
		uint32_t getRawAddr() const
		{
			return m_address;
		}

		/**
		 * @brief Creates IPv4 address from string.
		 * Implementation is platform-specific.
		 * @param string IPv4 address string.
		 * @return IPv4 address.
		 * @throws std::invalid_argument If the string does not contain valid IPv4 address.
		 */
		static AddressIP4 CreateFromString(const KString & string);
	};

	/**
	 * @brief IPv6 address.
	 */
	class AddressIP6 : public IAddress
	{
	public:
		using RawAddr = uint32_t[4];

	private:
		//! Entire 128-bit IPv6 address in network byte order (big-endian) divided into 4 blocks.
		RawAddr m_address;

	public:
		/**
		 * @brief Constructor.
		 * @param address Raw IPv6 address in network byte order.
		 */
		AddressIP6(const RawAddr & address)
		: m_address{ address[0], address[1], address[2], address[3] }
		{
		}

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		EAddressType getType() const override
		{
			return EAddressType::IP6;
		}

		/**
		 * @brief Returns name of address type.
		 * @return Address type name.
		 */
		KString getTypeName() const override
		{
			return "IPv6";
		}

		/**
		 * @brief Converts IPv6 address to string.
		 * No lookups are performed. Implementation is platform-specific.
		 * @return IPv6 address string.
		 */
		std::string toString() const override;

		/**
		 * @brief Copies raw IPv6 address to some buffer.
		 * Size of the buffer must be at least 16 bytes.
		 * @param buffer The buffer.
		 */
		void copyRawTo(void *buffer) const override
		{
			std::memcpy(buffer, &m_address, 16);
		}

		/**
		 * @brief Returns raw IPv6 address.
		 * @return Raw IPv6 address in network byte order.
		 */
		const RawAddr & getRawAddr() const
		{
			return m_address;
		}

		/**
		 * @brief Creates IPv6 address from string.
		 * Implementation is platform-specific.
		 * @param string IPv6 address string.
		 * @return IPv6 address.
		 * @throws std::invalid_argument If the string does not contain valid IPv6 address.
		 */
		static AddressIP6 CreateFromString(const KString & string);
	};

	/**
	 * @brief Container for multiple network addresses of different type.
	 */
	class AddressPack
	{
		std::vector<std::unique_ptr<IAddress>> m_pack;

	public:
		AddressPack()
		: m_pack()
		{
		}

		bool isEmpty() const
		{
			return m_pack.empty();
		}

		size_t getSize() const
		{
			return m_pack.size();
		}

		const IAddress & operator[](size_t index) const
		{
			return *m_pack[index];
		}

		IAddress & operator[](size_t index)
		{
			return *m_pack[index];
		}

		void clear()
		{
			m_pack.clear();
		}

		template<class T>
		void add(const T & address)
		{
			m_pack.emplace_back(std::make_unique<T>(address));
		}

		template<class T, class... Args>
		void emplace(Args &&... args)
		{
			m_pack.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		}
	};

	/**
	 * @brief Network address data.
	 */
	class AddressData
	{
		//! Data belongs to this address.
		const IAddress *m_pAddress;

	#ifndef CONNTOP_DEDICATED
		//! True if address hostname has been resolved, otherwise false.
		bool m_isHostnameResolved;
		//! True if ASN has been resolved, otherwise false.
		bool m_isASNResolved;
		//! True if address location has been resolved, otherwise false.
		bool m_isCountryResolved;
		//! True if address WHOIS data has been resolved, otherwise false.
		bool m_isWhoisAddressResolved;
		//! True if hostname WHOIS data has been resolved, otherwise false.
		bool m_isWhoisHostnameResolved;
		//! True if autonomous system WHOIS data has been resolved, otherwise false.
		bool m_isWhoisASResolved;

		//! Possibly empty string with address hostname.
		std::string m_hostname;
		//! Autonomous system number.
		ASN m_asn;
		//! Address location.
		Country m_country;
		//! WHOIS data.
		std::vector<WhoisData> m_whois;
	#endif

		//! Cached address string.
		std::string m_numericString;

	public:
		/**
		 * @brief Constructor.
		 * The address must be stored somewhere else because this class holds only a pointer to it.
		 * @param address The address.
		 */
		AddressData(const IAddress & address)
		: m_pAddress(&address),
	#ifndef CONNTOP_DEDICATED
		  m_isHostnameResolved(false),
		  m_isASNResolved(false),
		  m_isCountryResolved(false),
		  m_isWhoisAddressResolved(false),
		  m_isWhoisHostnameResolved(false),
		  m_isWhoisASResolved(false),
		  m_hostname(),
		  m_asn(),
		  m_country(),
		  m_whois(),
	#endif
		  m_numericString(address.toString())
		{
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Checks if hostname is resolved.
		 * @return True, if hostname has been resolved, otherwise false.
		 */
		bool isHostnameResolved() const
		{
			return m_isHostnameResolved;
		}

		/**
		 * @brief Checks if the address has any hostname.
		 * @return True, if the address has hostname, otherwise false.
		 */
		bool isHostnameAvailable() const
		{
			return !m_hostname.empty();
		}

		/**
		 * @brief Checks if autonomous system number is resolved.
		 * @return True, if autonomous system number has been resolved, otherwise false.
		 */
		bool isASNResolved() const
		{
			return m_isASNResolved;
		}

		/**
		 * @brief Checks if the address has any autonomous system number.
		 * @return True, if the address has autonomous system number, otherwise false.
		 */
		bool isASNAvailable() const
		{
			return !m_asn.isEmpty();
		}

		/**
		 * @brief Checks if country is resolved.
		 * @return True, if country has been resolved, otherwise false.
		 */
		bool isCountryResolved() const
		{
			return m_isCountryResolved;
		}

		/**
		 * @brief Checks if the address has any country.
		 * @return True, if the address has country, otherwise false.
		 */
		bool isCountryAvailable() const
		{
			return !m_country.isUnknown();
		}

		/**
		 * @brief Checks if address WHOIS data is resolved.
		 * @return True, if address WHOIS data has been resolved, otherwise false.
		 */
		bool isWhoisAddressResolved() const
		{
			return m_isWhoisAddressResolved;
		}

		/**
		 * @brief Checks if hostname WHOIS data is resolved.
		 * @return True, if hostname WHOIS data has been resolved, otherwise false.
		 */
		bool isWhoisHostnameResolved() const
		{
			return m_isWhoisHostnameResolved;
		}

		/**
		 * @brief Checks if autonomous system WHOIS data is resolved.
		 * @return True, if autonomous system WHOIS data has been resolved, otherwise false.
		 */
		bool isWhoisASResolved() const
		{
			return m_isWhoisASResolved;
		}

		/**
		 * @brief Checks if the address has WHOIS data of specified type.
		 * @param type WHOIS data type.
		 * @return True, if the address has the WHOIS data type, otherwise false.
		 */
		bool isWhoisAvailable(WhoisData::EType type) const
		{
			for (const WhoisData & data : m_whois)
			{
				if (data.getType() == type)
				{
					return !data.hasStatus() && !data.isEmpty();
				}
			}
			return false;
		}

		/**
		 * @brief Checks if the address has any address WHOIS data.
		 * @return True, if the address has address WHOIS data, otherwise false.
		 */
		bool isWhoisAddressAvailable() const
		{
			return isWhoisAvailable(WhoisData::IP_ADDRESS);
		}

		/**
		 * @brief Checks if the address has any hostname WHOIS data.
		 * @return True, if the address has hostname WHOIS data, otherwise false.
		 */
		bool isWhoisHostnameAvailable() const
		{
			return isWhoisAvailable(WhoisData::DOMAIN_NAME);
		}

		/**
		 * @brief Checks if the address has any autonomous system WHOIS data.
		 * @return True, if the address has autonomous system WHOIS data, otherwise false.
		 */
		bool isWhoisASAvailable() const
		{
			return isWhoisAvailable(WhoisData::AUTONOMOUS_SYSTEM);
		}
	#endif

		/**
		 * @brief Returns address to which the data belongs.
		 * @return The address.
		 */
		const IAddress & getAddress() const
		{
			return *m_pAddress;
		}

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		EAddressType getAddressType() const
		{
			return m_pAddress->getType();
		}

		/**
		 * @brief Returns name of address type.
		 * @return Address type name.
		 */
		KString getAddressTypeName() const
		{
			return m_pAddress->getTypeName();
		}

		/**
		 * @brief Returns address as numeric string.
		 * @return Address string.
		 */
		const std::string & getNumericString() const
		{
			return m_numericString;
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Returns address hostname.
		 * @return Hostname or empty string if the address has no hostname.
		 */
		const std::string & getHostnameString() const
		{
			return m_hostname;
		}
	#endif

		/**
		 * @brief Returns address string.
		 * @return Hostname or numeric string if the address has no hostname.
		 */
		const std::string & getString() const
		{
		#ifndef CONNTOP_DEDICATED
			return (isHostnameAvailable()) ? getHostnameString() : getNumericString();
		#else
			return getNumericString();
		#endif
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Returns address autonomous system number.
		 * @return Autonomous system number.
		 */
		const ASN & getASN() const
		{
			return m_asn;
		}

		/**
		 * @brief Returns address country.
		 * @return Country.
		 */
		const Country & getCountry() const
		{
			return m_country;
		}

		/**
		 * @brief Returns WHOIS data of specified type.
		 * @param type WHOIS data type.
		 * @return WHOIS data.
		 */
		const WhoisData & getWhois(WhoisData::EType type) const
		{
			for (const WhoisData & data : m_whois)
			{
				if (data.getType() == type)
				{
					return data;
				}
			}
			return WhoisData::GetEmptyUnknown();
		}

		/**
		 * @brief Returns address WHOIS data.
		 * @return WHOIS data.
		 */
		const WhoisData & getWhoisAddress() const
		{
			return getWhois(WhoisData::IP_ADDRESS);
		}

		/**
		 * @brief Returns hostname WHOIS data.
		 * @return WHOIS data.
		 */
		const WhoisData & getWhoisHostname() const
		{
			return getWhois(WhoisData::DOMAIN_NAME);
		}

		/**
		 * @brief Returns autonomous system WHOIS data.
		 * @return WHOIS data.
		 */
		const WhoisData & getWhoisAS() const
		{
			return getWhois(WhoisData::AUTONOMOUS_SYSTEM);
		}

		/**
		 * @brief Sets address hostname.
		 * @param hostname Address hostname.
		 */
		void setResolvedHostname(std::string && hostname)
		{
			m_hostname = std::move(hostname);
			m_isHostnameResolved = true;
		}

		/**
		 * @brief Sets address autonomous system number.
		 * @param asn Address autonomous system number.
		 */
		void setResolvedASN(ASN && asn)
		{
			m_asn = std::move(asn);
			m_isASNResolved = true;
		}

		/**
		 * @brief Sets address country.
		 * @param country Address country.
		 */
		void setResolvedCountry(Country && country)
		{
			m_country = std::move(country);
			m_isCountryResolved = true;
		}

		/**
		 * @brief Sets WHOIS data.
		 * @param data WHOIS data.
		 */
		void setWhois(WhoisData && data)
		{
			switch (data.getType())
			{
				case WhoisData::UNKNOWN:
				{
					return;
				}
				case WhoisData::IP_ADDRESS:
				{
					m_isWhoisAddressResolved = true;
					break;
				}
				case WhoisData::DOMAIN_NAME:
				{
					m_isWhoisHostnameResolved = true;
					break;
				}
				case WhoisData::AUTONOMOUS_SYSTEM:
				{
					m_isWhoisASResolved = true;
					break;
				}
			}

			bool isReplaced = false;
			for (auto it = m_whois.begin(); it != m_whois.end(); ++it)
			{
				if (it->getType() == data.getType())
				{
					(*it) = std::move(data);
					isReplaced = true;
					break;
				}
			}

			if (!isReplaced)
			{
				m_whois.emplace_back(std::move(data));
			}
		}
	#endif

		/**
		 * @brief Sets address to which the data belongs.
		 * This function should be used only in ConnectionStorage class.
		 * @param address The address.
		 */
		void setAddress(const IAddress & address)
		{
			m_pAddress = &address;
		}
	};

	inline bool operator==(const AddressIP4 & a, const AddressIP4 & b)
	{
		return a.getRawAddr() == b.getRawAddr();
	}

	inline bool operator!=(const AddressIP4 & a, const AddressIP4 & b)
	{
		return !(a == b);
	}

	inline bool operator==(const AddressIP6 & a, const AddressIP6 & b)
	{
		const AddressIP6::RawAddr & aAddr = a.getRawAddr();
		const AddressIP6::RawAddr & bAddr = b.getRawAddr();
		return aAddr[3] == bAddr[3]
		    && aAddr[2] == bAddr[2]
		    && aAddr[1] == bAddr[1]
		    && aAddr[0] == bAddr[0];
	}

	inline bool operator!=(const AddressIP6 & a, const AddressIP6 & b)
	{
		return !(a == b);
	}
}

namespace std
{
	template<>
	struct hash<ctp::AddressIP4>
	{
		using argument_type = ctp::AddressIP4;
		using result_type = size_t;

		result_type operator()(const argument_type & v) const
		{
			return hash<uint32_t>()(v.getRawAddr());
		}
	};

	template<>
	struct hash<ctp::AddressIP6>
	{
		using argument_type = ctp::AddressIP6;
		using result_type = size_t;

		result_type operator()(const argument_type & v) const
		{
			using ctp::HashCombine;
			using ctp::AddressIP6;

			const AddressIP6::RawAddr & address = v.getRawAddr();
			result_type h = hash<uint32_t>()(address[0]);
			HashCombine(h, hash<uint32_t>()(address[1]));
			HashCombine(h, hash<uint32_t>()(address[2]));
			HashCombine(h, hash<uint32_t>()(address[3]));

			return h;
		}
	};
}
