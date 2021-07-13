/**
 * @file
 * @brief Network port classes.
 */

#pragma once

#include <string>
#include <vector>

#include "Types.hpp"
#include "KString.hpp"
#include "Hash.hpp"
#include "conntop_config.h"

namespace ctp
{
	/**
	 * @brief Network port type (L4 protocol).
	 */
	enum struct EPortType
	{
		UDP,  //!< UDP port
		TCP   //!< TCP port
	};

	/**
	 * @brief Network port.
	 */
	class Port
	{
		//! Port type.
		EPortType m_type;
		//! Port number in system byte order.
		uint16_t m_number;

	public:
		/**
		 * @brief Constructor.
		 * @param type Port type.
		 * @param number Port number in system byte order.
		 */
		Port(EPortType type, uint16_t number)
		: m_type(type),
		  m_number(number)
		{
		}

		/**
		 * @brief Returns port type.
		 * @return Port type.
		 */
		EPortType getType() const
		{
			return m_type;
		}

		/**
		 * @brief Returns name of port type.
		 * @return Port type name.
		 */
		KString getTypeName() const
		{
			switch (m_type)
			{
				case EPortType::UDP: return "UDP";
				case EPortType::TCP: return "TCP";
			}
			return "?";
		}

		/**
		 * @brief Returns port number in system byte order.
		 * @return Port number.
		 */
		uint16_t getNumber() const
		{
			return m_number;
		}
	};

	/**
	 * @brief Container for multiple network ports.
	 */
	class PortPack
	{
		std::vector<Port> m_pack;

	public:
		PortPack()
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

		const Port & operator[](size_t index) const
		{
			return m_pack[index];
		}

		Port & operator[](size_t index)
		{
			return m_pack[index];
		}

		void clear()
		{
			m_pack.clear();
		}

		void add(const Port & port)
		{
			m_pack.push_back(port);
		}

		template<class... Args>
		void emplace(Args &&... args)
		{
			m_pack.emplace_back(std::forward<Args>(args)...);
		}
	};

	/**
	 * @brief Network port data.
	 */
	class PortData
	{
		//! Data belongs to this port.
		const Port *m_pPort;

	#ifndef CONNTOP_DEDICATED
		//! True if port service name has been resolved, otherwise false.
		bool m_isServiceResolved;

		//! Possibly empty string with service name.
		std::string m_service;
	#endif

		//! Cached port number string.
		std::string m_numeric;

	public:
		/**
		 * @brief Constructor.
		 * The port must be stored somewhere else because this class holds only a pointer to it.
		 * @param port The port.
		 */
		PortData(const Port & port)
		: m_pPort(&port),
	#ifndef CONNTOP_DEDICATED
		  m_isServiceResolved(false),
		  m_service(),
	#endif
		  m_numeric(std::to_string(port.getNumber()))
		{
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Checks if service name is resolved.
		 * @return True, if service name has been resolved, otherwise false.
		 */
		bool isServiceResolved() const
		{
			return m_isServiceResolved;
		}

		/**
		 * @brief Checks if the port has any service name.
		 * @return True, if the port has service name, otherwise false.
		 */
		bool isServiceAvailable() const
		{
			return !m_service.empty();
		}
	#endif

		/**
		 * @brief Returns port to which the data belongs.
		 * @return The port.
		 */
		const Port & getPort() const
		{
			return *m_pPort;
		}

		/**
		 * @brief Returns port type.
		 * @return Port type.
		 */
		EPortType getPortType() const
		{
			return m_pPort->getType();
		}

		/**
		 * @brief Returns name of port type.
		 * @return Port type name.
		 */
		KString getPortTypeName() const
		{
			return m_pPort->getTypeName();
		}

		/**
		 * @brief Returns port number in system byte order.
		 * @return Port number.
		 */
		uint16_t getPortNumber() const
		{
			return m_pPort->getNumber();
		}

		/**
		 * @brief Returns port number as string.
		 * @return Port number string.
		 */
		const std::string & getNumericString() const
		{
			return m_numeric;
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Returns port service name.
		 * @return Service name or empty string if the port has no service name.
		 */
		const std::string & getServiceString() const
		{
			return m_service;
		}
	#endif

		/**
		 * @brief Returns port string.
		 * @return Service name or port number as string if the port has no service name.
		 */
		const std::string & getString() const
		{
		#ifndef CONNTOP_DEDICATED
			return (isServiceAvailable()) ? getServiceString() : getNumericString();
		#else
			return getNumericString();
		#endif
		}

	#ifndef CONNTOP_DEDICATED
		/**
		 * @brief Sets port service name.
		 * @param service Port service name.
		 */
		void setResolvedService(std::string && service)
		{
			m_service = std::move(service);
			m_isServiceResolved = true;
		}
	#endif

		/**
		 * @brief Sets port to which the data belongs.
		 * This function should be used only in ConnectionStorage class.
		 * @param port The port.
		 */
		void setPort(const Port & port)
		{
			m_pPort = &port;
		}
	};

	inline bool operator==(const Port & a, const Port & b)
	{
		return a.getType() == b.getType() && a.getNumber() == b.getNumber();
	}

	inline bool operator!=(const Port & a, const Port & b)
	{
		return !(a == b);
	}
}

namespace std
{
	template<>
	struct hash<ctp::Port>
	{
		using argument_type = ctp::Port;
		using result_type = size_t;

		result_type operator()(const argument_type & v) const
		{
			using ctp::HashCombine;

			result_type h = hash<uint16_t>()(v.getNumber());
			HashCombine(h, hash<int>()(static_cast<int>(v.getType())));

			return h;
		}
	};
}
