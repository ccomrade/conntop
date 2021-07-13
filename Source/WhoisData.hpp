/**
 * @file
 * @brief WhoisData class.
 */

#pragma once

#include <string>

namespace ctp
{
	/**
	 * @brief Data received from WHOIS server.
	 */
	class WhoisData
	{
	public:
		enum EType
		{
			UNKNOWN,
			IP_ADDRESS,
			DOMAIN_NAME,
			AUTONOMOUS_SYSTEM
		};

	private:
		EType m_type;
		bool m_hasStatus;
		std::string m_data;

	public:
		WhoisData(EType type = UNKNOWN)
		: m_type(type),
		  m_hasStatus(false),
		  m_data()
		{
		}

		WhoisData(EType type, std::string && data, bool hasStatus = false)
		: m_type(type),
		  m_hasStatus(hasStatus),
		  m_data(std::move(data))
		{
		}

		WhoisData(const WhoisData &) = default;

		WhoisData(WhoisData && other)
		: m_type(other.m_type),
		  m_hasStatus(other.m_hasStatus),
		  m_data(std::move(other.m_data))
		{
			other.m_type = UNKNOWN;
			other.m_hasStatus = false;
		}

		WhoisData & operator=(const WhoisData &) = default;

		WhoisData & operator=(WhoisData && other)
		{
			if (this != &other)
			{
				m_type = other.m_type;
				m_hasStatus = other.m_hasStatus;
				m_data = std::move(other.m_data);
				other.m_type = UNKNOWN;
				other.m_hasStatus = false;
			}
			return *this;
		}

		EType getType() const
		{
			return m_type;
		}

		bool isEmpty() const
		{
			return m_data.empty();
		}

		bool hasStatus() const
		{
			return m_hasStatus;
		}

		const std::string & getData() const
		{
			return m_data;
		}

		static const WhoisData & GetEmptyUnknown();
	};
}
