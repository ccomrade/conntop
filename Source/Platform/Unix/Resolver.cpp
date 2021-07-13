/**
 * @file
 * @brief Implementation of platform-specific functions from Resolver class for Unix platform.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Resolver.hpp"
#include "GetAddrInfo.hpp"
#include "Log.hpp"

AddressPack Resolver::PlatformResolveHostname(const KString & hostname)
{
	GetAddrInfo info(hostname.c_str(), nullptr, AF_UNSPEC);

	if (info.hasError())
	{
		if (gLog->isMsgEnabled(Log::NOTICE))
		{
			gLog->notice("[Resolver] Hostname '%s' cannot be resolved: %s",
			  hostname.c_str(),
			  info.getErrorString().c_str()
			);
		}
		return AddressPack();
	}

	AddressPack pack;
	while (!info.isEmpty())
	{
		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				pack.emplace<AddressIP4>(pAddr->sin_addr.s_addr);
				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				pack.emplace<AddressIP6>(pAddr->sin6_addr.s6_addr32);
				break;
			}
		}

		info.next();
	}

	return pack;
}

PortPack Resolver::PlatformResolveService(const KString & service, EPortType portType)
{
	int addressFamily = AF_INET;  // IPv4 and IPv6 have same services
	int socketType = 0;
	int socketProtocol = 0;

	switch (portType)
	{
		case EPortType::UDP:
		{
			socketType = SOCK_DGRAM;
			socketProtocol = IPPROTO_UDP;
			break;
		}
		case EPortType::TCP:
		{
			socketType = SOCK_STREAM;
			socketProtocol = IPPROTO_TCP;
			break;
		}
	}

	GetAddrInfo info(nullptr, service.c_str(), addressFamily, socketType, socketProtocol);

	if (info.hasError())
	{
		if (gLog->isMsgEnabled(Log::NOTICE))
		{
			gLog->notice("[Resolver] Service '%s' cannot be resolved: %s",
			  service.c_str(),
			  info.getErrorString().c_str()
			);
		}
		return PortPack();
	}

	PortPack pack;
	while (!info.isEmpty())
	{
		switch (info->ai_family)
		{
			case AF_INET:
			{
				const sockaddr_in *pAddr = reinterpret_cast<const sockaddr_in*>(info->ai_addr);
				pack.emplace(portType, ntohs(pAddr->sin_port));
				break;
			}
			case AF_INET6:
			{
				const sockaddr_in6 *pAddr = reinterpret_cast<const sockaddr_in6*>(info->ai_addr);
				pack.emplace(portType, ntohs(pAddr->sin6_port));
				break;
			}
		}

		info.next();
	}

	return pack;
}

std::string Resolver::PlatformResolveAddress(const IAddress & address)
{
	sockaddr_storage addr{};
	socklen_t size;
	switch (address.getType())
	{
		case EAddressType::IP4:
		{
			sockaddr_in *pAddr = reinterpret_cast<sockaddr_in*>(&addr);
			pAddr->sin_family = AF_INET;
			address.copyRawTo(&pAddr->sin_addr);
			size = sizeof (sockaddr_in);
			break;
		}
		case EAddressType::IP6:
		{
			sockaddr_in6 *pAddr = reinterpret_cast<sockaddr_in6*>(&addr);
			pAddr->sin6_family = AF_INET6;
			address.copyRawTo(&pAddr->sin6_addr);
			size = sizeof (sockaddr_in6);
			break;
		}
		default:
		{
			gLog->error("[Resolver] Unable to resolve address of unknown type %s",
			  address.getTypeName().c_str()
			);
			return std::string();
		}
	}

	const int flags = NI_NAMEREQD;
	const sockaddr *pAddr = reinterpret_cast<const sockaddr*>(&addr);

	char buffer[NI_MAXHOST];
	if (int status = getnameinfo(pAddr, size, buffer, sizeof buffer, nullptr, 0, flags))
	{
		if (gLog->isMsgEnabled(Log::NOTICE))
		{
			gLog->notice("[Resolver] Hostname of %s address %s cannot be resolved: %s",
			  address.getTypeName().c_str(),
			  address.toString().c_str(),
			  gai_strerror(status)
			);
		}
		return std::string();
	}

	return std::string(buffer);
}

std::string Resolver::PlatformResolvePort(const Port & port)
{
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port.getNumber());
	socklen_t size = sizeof addr;

	const int flags = (port.getType() == EPortType::UDP) ? NI_DGRAM : 0;
	const sockaddr *pAddr = reinterpret_cast<const sockaddr*>(&addr);

	char buffer[NI_MAXSERV];
	if (int status = getnameinfo(pAddr, size, nullptr, 0, buffer, sizeof buffer, flags))
	{
		if (gLog->isMsgEnabled(Log::NOTICE))
		{
			gLog->notice("[Resolver] Service of %s port %hu cannot be resolved: %s",
			  port.getTypeName().c_str(),
			  port.getNumber(),
			  gai_strerror(status)
			);
		}
		return std::string();
	}

	std::string serviceName(buffer);

	// if no service name is found, getnameinfo silently converts port number to string
	if (serviceName == std::to_string(port.getNumber()))
	{
		serviceName.clear();
	}

	return serviceName;
}
