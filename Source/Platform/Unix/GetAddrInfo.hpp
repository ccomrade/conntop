/**
 * @file
 * @brief GetAddrInfo class.
 */

#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>

#include "Util.hpp"

namespace ctp
{
	template<int Flags = 0>
	class GenericGetAddrInfo
	{
		addrinfo *m_pAddrInfo;
		addrinfo *m_pCurrentInfo;
		int m_status;

	public:
		GenericGetAddrInfo( const char *node, const char *service, int family, int type = SOCK_STREAM, int protocol = 0 )
		: m_pAddrInfo(nullptr),
		  m_pCurrentInfo(nullptr),
		  m_status()
		{
			addrinfo hints{};
			hints.ai_flags = Flags;
			hints.ai_family = family;
			hints.ai_socktype = type;
			hints.ai_protocol = protocol;

			m_status = getaddrinfo( node, service, &hints, &m_pAddrInfo );

			if ( m_status == 0 )
			{
				m_pCurrentInfo = m_pAddrInfo;
			}
		}

		~GenericGetAddrInfo()
		{
			if ( m_pAddrInfo )
			{
				freeaddrinfo( m_pAddrInfo );
			}
		}

		bool hasError() const
		{
			return m_status != 0;
		}

		bool isEmpty() const
		{
			return m_pCurrentInfo == nullptr;
		}

		const addrinfo *operator->() const
		{
			return m_pCurrentInfo;
		}

		const addrinfo *get() const
		{
			return m_pCurrentInfo;
		}

		void next()
		{
			if ( m_pCurrentInfo )
			{
				m_pCurrentInfo = m_pCurrentInfo->ai_next;
			}
		}

		int getErrorNumber() const
		{
			return m_status;
		}

		std::string getErrorString() const
		{
			return (m_status == EAI_SYSTEM) ? Util::ErrnoToString() : gai_strerror( m_status );
		}
	};

	using GetAddrInfo = GenericGetAddrInfo<>;

	using GetAddrInfoNumeric = GenericGetAddrInfo<AI_NUMERICHOST | AI_NUMERICSERV>;
}
