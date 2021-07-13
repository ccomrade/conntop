/**
 * @file
 * @brief Implementation of socket classes for Unix platform.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "Sockets.hpp"
#include "GetAddrInfo.hpp"  // GetAddrInfoNumeric
#include "Log.hpp"

namespace ctp
{
	static constexpr int LISTEN_BACKLOG_SIZE = 64;

	static int GetAddressFamilyFromType(EAddressType addressType)
	{
		switch (addressType)
		{
			case EAddressType::IP4:
			{
				return AF_INET;
			}
			case EAddressType::IP6:
			{
				return AF_INET6;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	static int GetSocketProtocolFromType(EStreamSocketType socketType)
	{
		switch (socketType)
		{
			case EStreamSocketType::TCP:
			{
				return IPPROTO_TCP;
			}
			default:
			{
				throw SocketException(EPROTONOSUPPORT);
			}
		}
	}

	static SocketEndpoint GetSocketEndpoint(int fd, EPortType portType, bool remote)
	{
		sockaddr_storage addr{};
		socklen_t addrSize = sizeof addr;
		if (remote)
		{
			if (getpeername(fd, reinterpret_cast<sockaddr*>(&addr), &addrSize) < 0)
			{
				throw SocketException(errno);
			}
		}
		else
		{
			if (getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &addrSize) < 0)
			{
				throw SocketException(errno);
			}
		}

		std::unique_ptr<IAddress> pAddress;
		std::unique_ptr<Port> pPort;
		switch (addr.ss_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(&addr);
				pAddress = std::make_unique<AddressIP4>(pAddr->sin_addr.s_addr);
				pPort = std::make_unique<Port>(portType, ntohs(pAddr->sin_port));
				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(&addr);
				pAddress = std::make_unique<AddressIP6>(pAddr->sin6_addr.s6_addr32);
				pPort = std::make_unique<Port>(portType, ntohs(pAddr->sin6_port));
				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}

		return SocketEndpoint(std::move(pAddress), std::move(pPort));
	}

	static KString SocketTypeToString(EStreamSocketType socketType)
	{
		switch (socketType)
		{
			case EStreamSocketType::TCP: return "TCP";
		}

		return "?";
	}

	static bool InitSocket(int fd)
	{
		if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
			return false;

		if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
			return false;

		return true;
	}

	void StreamSocket::connect(const IAddress & address, uint16_t port, EStreamSocketType type)
	{
		if (isConnected())
		{
			throw SocketException(EISCONN);
		}

		const int socketProtocol = GetSocketProtocolFromType(type);
		const KString protoName = SocketTypeToString(type);
		m_type = type;

		switch (address.getType())
		{
			case EAddressType::IP4:
			{
				m_fd = ::socket(AF_INET, SOCK_STREAM, socketProtocol);
				if (m_fd < 0)
				{
					throw SocketException(errno);
				}

				if (!InitSocket(m_fd))
				{
					close_nothrow();
					throw SocketException(errno);
				}

				gLog->info("[StreamSocket] Created IPv4 %s socket on %d", protoName.c_str(), m_fd);

				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				address.copyRawTo(&addr.sin_addr);
				if (::connect(m_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof addr) < 0)
				{
					if (errno != EINPROGRESS)
					{
						close_nothrow();
						throw SocketException(errno);
					}
				}

				break;
			}
			case EAddressType::IP6:
			{
				m_fd = ::socket(AF_INET6, SOCK_STREAM, socketProtocol);
				if (m_fd < 0)
				{
					throw SocketException(errno);
				}

				if (!InitSocket(m_fd))
				{
					close_nothrow();
					throw SocketException(errno);
				}

				const int v6Only = 1;
				if (::setsockopt(m_fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof v6Only) < 0)
				{
					close_nothrow();
					throw SocketException(errno);
				}

				gLog->info("[StreamSocket] Created IPv6 %s socket on %d", protoName.c_str(), m_fd);

				sockaddr_in6 addr{};
				addr.sin6_family = AF_INET6;
				addr.sin6_port = htons(port);
				address.copyRawTo(&addr.sin6_addr);
				if (::connect(m_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof addr) < 0)
				{
					if (errno != EINPROGRESS)
					{
						close_nothrow();
						throw SocketException(errno);
					}
				}

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}

		if (gLog->isMsgEnabled(Log::INFO))
		{
			const std::string endpointString = Util::AddressPortToString(address, port);

			gLog->info("[StreamSocket] Connecting to %s on %d", endpointString.c_str(), m_fd);
		}
	}

	void StreamSocket::verifyConnect()
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		int errorNumber;
		socklen_t errorNumberSize = sizeof errorNumber;
		if (::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &errorNumber, &errorNumberSize) < 0)
		{
			throw SocketException(errno);
		}

		if (errorNumber != 0)
		{
			throw SocketException(errorNumber);
		}
	}

	void StreamSocket::close()
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		int status = ::close(m_fd);

		gLog->info("[StreamSocket] Closed socket on %d", m_fd);

		m_fd = -1;

		if (status < 0)
		{
			throw SocketException(errno);
		}
	}

	bool StreamSocket::close_nothrow()
	{
		if (isConnected())
		{
			int oldErrno = errno;

			int status = ::close(m_fd);

			gLog->info("[StreamSocket] Closed socket on %d", m_fd);

			m_fd = -1;

			if (status == 0)
			{
				return true;
			}

			errno = oldErrno;
		}

		return false;
	}

	size_t StreamSocket::send(const char *data, size_t dataLength)
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		ssize_t length = ::send(m_fd, data, dataLength, 0);
		if (length < 0)
		{
			throw SocketException(errno);
		}

		return length;
	}

	size_t StreamSocket::receive(char *buffer, size_t bufferSize)
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		ssize_t length = ::recv(m_fd, buffer, bufferSize, 0);
		if (length < 0)
		{
			throw SocketException(errno);
		}

		return length;
	}

	int StreamSocket::getType() const
	{
		if (!isConnected())
		{
			return -1;
		}

		return static_cast<int>(m_type);
	}

	KString StreamSocket::getTypeName() const
	{
		return SocketTypeToString(static_cast<EStreamSocketType>(getType()));
	}

	SocketEndpoint StreamSocket::getLocalEndpoint() const
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		switch (m_type)
		{
			case EStreamSocketType::TCP:
			{
				return GetSocketEndpoint(m_fd, EPortType::TCP, false);
			}
			default:
			{
				throw SocketException(EPROTONOSUPPORT);
			}
		}
	}

	SocketEndpoint StreamSocket::getRemoteEndpoint() const
	{
		if (!isConnected())
		{
			throw SocketException(ENOTCONN);
		}

		switch (m_type)
		{
			case EStreamSocketType::TCP:
			{
				return GetSocketEndpoint(m_fd, EPortType::TCP, true);
			}
			default:
			{
				throw SocketException(EPROTONOSUPPORT);
			}
		}
	}

	void StreamServerSocket::open(const IAddress & address, uint16_t port, EStreamSocketType type)
	{
		if (isOpen())
		{
			throw SocketException(EISCONN);
		}

		const int socketProtocol = GetSocketProtocolFromType(type);
		const KString protoName = SocketTypeToString(type);
		m_type = type;

		switch (address.getType())
		{
			case EAddressType::IP4:
			{
				m_fd = ::socket(AF_INET, SOCK_STREAM, socketProtocol);
				if (m_fd < 0)
				{
					throw SocketException(errno);
				}

				if (!InitSocket(m_fd))
				{
					close_nothrow();
					throw SocketException(errno);
				}

				gLog->info("[StreamServerSocket] Created IPv4 %s socket on %d", protoName.c_str(), m_fd);

				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				address.copyRawTo(&addr.sin_addr);
				if (::bind(m_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof addr) < 0)
				{
					close_nothrow();
					throw SocketException(errno);
				}

				break;
			}
			case EAddressType::IP6:
			{
				m_fd = ::socket(AF_INET6, SOCK_STREAM, socketProtocol);
				if (m_fd < 0)
				{
					throw SocketException(errno);
				}

				if (!InitSocket(m_fd))
				{
					close_nothrow();
					throw SocketException(errno);
				}

				const int v6Only = 1;
				if (::setsockopt(m_fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof v6Only) < 0)
				{
					close_nothrow();
					throw SocketException(errno);
				}

				gLog->info("[StreamServerSocket] Created IPv6 %s socket on %d", protoName.c_str(), m_fd);

				sockaddr_in6 addr{};
				addr.sin6_family = AF_INET6;
				addr.sin6_port = htons(port);
				address.copyRawTo(&addr.sin6_addr);
				if (::bind(m_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof addr) < 0)
				{
					close_nothrow();
					throw SocketException(errno);
				}

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}

		if (::listen(m_fd, LISTEN_BACKLOG_SIZE) < 0)
		{
			close_nothrow();
			throw SocketException(errno);
		}

		if (gLog->isMsgEnabled(Log::INFO))
		{
			const std::string endpointString = Util::AddressPortToString(address, port);

			gLog->info("[StreamServerSocket] Socket on %d is listening on %s", m_fd, endpointString.c_str());
		}
	}

	void StreamServerSocket::open(const IAddress & address, const KString & portString, EStreamSocketType type)
	{
		const int addressFamily = GetAddressFamilyFromType(address.getType());
		const int socketProtocol = GetSocketProtocolFromType(type);

		GetAddrInfoNumeric info(nullptr, portString.c_str(), addressFamily, SOCK_STREAM, socketProtocol);

		if (info.hasError())
		{
			throw SocketException(info.getErrorNumber(), info.getErrorString());
		}

		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				const uint16_t port = ntohs(pAddr->sin_port);

				open(address, port, type);

				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				const uint16_t port = ntohs(pAddr->sin6_port);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::open(const KString & addressString, uint16_t port, EStreamSocketType type)
	{
		const int socketProtocol = GetSocketProtocolFromType(type);

		GetAddrInfoNumeric info(addressString.c_str(), nullptr, AF_UNSPEC, SOCK_STREAM, socketProtocol);

		if (info.hasError())
		{
			throw SocketException(info.getErrorNumber(), info.getErrorString());
		}

		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				const AddressIP4 address(pAddr->sin_addr.s_addr);

				open(address, port, type);

				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				const AddressIP6 address(pAddr->sin6_addr.s6_addr32);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::open(const KString & addressString, const KString & portString, EStreamSocketType type)
	{
		const int socketProtocol = GetSocketProtocolFromType(type);

		GetAddrInfoNumeric info(addressString.c_str(), portString.c_str(), AF_UNSPEC, SOCK_STREAM, socketProtocol);

		if (info.hasError())
		{
			throw SocketException(info.getErrorNumber(), info.getErrorString());
		}

		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				const AddressIP4 address(pAddr->sin_addr.s_addr);
				const uint16_t port = ntohs(pAddr->sin_port);

				open(address, port, type);

				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				const AddressIP6 address(pAddr->sin6_addr.s6_addr32);
				const uint16_t port = ntohs(pAddr->sin6_port);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::openAny(EAddressType addressType, uint16_t port, EStreamSocketType type)
	{
		switch (addressType)
		{
			case EAddressType::IP4:
			{
				const AddressIP4 address(htonl(INADDR_ANY));

				open(address, port, type);

				break;
			}
			case EAddressType::IP6:
			{
				const AddressIP6 address(in6addr_any.s6_addr32);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::openAny(EAddressType addressType, const KString & portString, EStreamSocketType type)
	{
		const int addressFamily = GetAddressFamilyFromType(addressType);
		const int socketProtocol = GetSocketProtocolFromType(type);

		GetAddrInfoNumeric info(nullptr, portString.c_str(), addressFamily, SOCK_STREAM, socketProtocol);

		if (info.hasError())
		{
			throw SocketException(info.getErrorNumber(), info.getErrorString());
		}

		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				const AddressIP4 address(htonl(INADDR_ANY));
				const uint16_t port = ntohs(pAddr->sin_port);

				open(address, port, type);

				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				const AddressIP6 address(in6addr_any.s6_addr32);
				const uint16_t port = ntohs(pAddr->sin6_port);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::openLocalhost(EAddressType addressType, uint16_t port, EStreamSocketType type)
	{
		switch (addressType)
		{
			case EAddressType::IP4:
			{
				const AddressIP4 address(htonl(INADDR_LOOPBACK));

				open(address, port, type);

				break;
			}
			case EAddressType::IP6:
			{
				const AddressIP6 address(in6addr_loopback.s6_addr32);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::openLocalhost(EAddressType addressType, const KString & portString, EStreamSocketType type)
	{
		const int addressFamily = GetAddressFamilyFromType(addressType);
		const int socketProtocol = GetSocketProtocolFromType(type);

		GetAddrInfoNumeric info(nullptr, portString.c_str(), addressFamily, SOCK_STREAM, socketProtocol);

		if (info.hasError())
		{
			throw SocketException(info.getErrorNumber(), info.getErrorString());
		}

		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				const AddressIP4 address(htonl(INADDR_LOOPBACK));
				const uint16_t port = ntohs(pAddr->sin_port);

				open(address, port, type);

				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				const AddressIP6 address(in6addr_loopback.s6_addr32);
				const uint16_t port = ntohs(pAddr->sin6_port);

				open(address, port, type);

				break;
			}
			default:
			{
				throw SocketException(EAFNOSUPPORT);
			}
		}
	}

	void StreamServerSocket::close()
	{
		if (!isOpen())
		{
			throw SocketException(ENOTCONN);
		}

		int status = ::close(m_fd);

		gLog->info("[StreamServerSocket] Closed socket on %d", m_fd);

		m_fd = -1;

		if (status < 0)
		{
			throw SocketException(errno);
		}
	}

	bool StreamServerSocket::close_nothrow()
	{
		if (isOpen())
		{
			int oldErrno = errno;

			int status = ::close(m_fd);

			gLog->info("[StreamServerSocket] Closed socket on %d", m_fd);

			m_fd = -1;

			if (status == 0)
			{
				return true;
			}

			errno = oldErrno;
		}

		return false;
	}

	StreamSocket StreamServerSocket::accept()
	{
		if (!isOpen())
		{
			throw SocketException(ENOTCONN);
		}

		const bool log = gLog->isMsgEnabled(Log::INFO);

		int socketFD;
		sockaddr_storage addr{};
		socklen_t addrSize = sizeof addr;

		if (log)
		{
			socketFD = ::accept(m_fd, reinterpret_cast<sockaddr*>(&addr), &addrSize);
		}
		else
		{
			socketFD = ::accept(m_fd, nullptr, nullptr);
		}

		if (socketFD < 0)
		{
			throw SocketException(errno);
		}

		if (!InitSocket(socketFD))
		{
			int errorNumber = errno;
			::close(socketFD);
			throw SocketException(errorNumber);
		}

		if (log)
		{
			KString familyName = "?";
			std::string endpointString;

			switch (addr.ss_family)
			{
				case AF_INET:
				{
					const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(&addr);
					const AddressIP4 address(pAddr->sin_addr.s_addr);
					const uint16_t port = ntohs(pAddr->sin_port);

					familyName = "IPv4";
					endpointString = Util::AddressPortToString(address, port);

					break;
				}
				case AF_INET6:
				{
					const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(&addr);
					const AddressIP6 address(pAddr->sin6_addr.s6_addr32);
					const uint16_t port = ntohs(pAddr->sin6_port);

					familyName = "IPv6";
					endpointString = Util::AddressPortToString(address, port);

					break;
				}
			}

			gLog->info("[StreamServerSocket] New %s %s connection %d from %s on %d",
			  familyName.c_str(),
			  getTypeName().c_str(),
			  socketFD,
			  endpointString.c_str(),
			  m_fd
			);
		}

		return StreamSocket(socketFD, m_type);
	}

	int StreamServerSocket::getType() const
	{
		if (!isOpen())
		{
			return -1;
		}

		return static_cast<int>(m_type);
	}

	KString StreamServerSocket::getTypeName() const
	{
		return SocketTypeToString(static_cast<EStreamSocketType>(getType()));
	}

	SocketEndpoint StreamServerSocket::getListenEndpoint() const
	{
		if (!isOpen())
		{
			throw SocketException(ENOTCONN);
		}

		switch (m_type)
		{
			case EStreamSocketType::TCP:
			{
				return GetSocketEndpoint(m_fd, EPortType::TCP, false);
			}
			default:
			{
				throw SocketException(EPROTONOSUPPORT);
			}
		}
	}
}
