#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>

#include "TCPServerSocket.h"
#include "System.h"

void TCPServerSocket::Open(const IPAddress& address, std::uint16_t port)
{
	////////////////////////////////////////////////////////////////////////////////

	Close();

	////////////////////////////////////////////////////////////////////////////////

	const int addressFamily = [&address]()
	{
		switch (address.type)
		{
			case IPAddressType::IPv4: return AF_INET;
			case IPAddressType::IPv6: return AF_INET6;
		}

		return -1;
	}();

	////////////////////////////////////////////////////////////////////////////////

	m_handle = Handle(socket(addressFamily, SOCK_STREAM, IPPROTO_TCP));
	if (!m_handle)
	{
		throw std::system_error(errno, std::system_category(), "socket");
	}

	System::SetFileDescriptorNonBlocking(m_handle.GetFileDescriptor());
	System::SetFileDescriptorCloseOnExec(m_handle.GetFileDescriptor());

	////////////////////////////////////////////////////////////////////////////////

	if (address.type == IPAddressType::IPv6)
	{
		const int v6Only = 1;

		if (setsockopt(m_handle.GetFileDescriptor(), IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof v6Only) < 0)
		{
			throw std::system_error(errno, std::system_category(), "setsockopt");
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	const sockaddr_storage socketAddress = [&address, port]()
	{
		sockaddr_storage result = {};

		switch (address.type)
		{
			case IPAddressType::IPv4:
			{
				auto sa4 = reinterpret_cast<sockaddr_in*>(&result);

				sa4->sin_family = AF_INET;
				sa4->sin_port = htons(port);
				std::memcpy(&sa4->sin_addr, &address.value, sizeof sa4->sin_addr);

				break;
			}
			case IPAddressType::IPv6:
			{
				auto sa6 = reinterpret_cast<sockaddr_in6*>(&result);

				sa6->sin6_family = AF_INET6;
				sa6->sin6_port = htons(port);
				std::memcpy(&sa6->sin6_addr, &address.value, sizeof sa6->sin6_addr);

				break;
			}
		}

		return result;
	}();

	////////////////////////////////////////////////////////////////////////////////

	if (bind(m_handle.GetFileDescriptor(), reinterpret_cast<const sockaddr*>(&socketAddress), sizeof socketAddress) < 0)
	{
		throw std::system_error(errno, std::system_category(), "bind");
	}

	////////////////////////////////////////////////////////////////////////////////

	constexpr int LISTEN_BACKLOG_SIZE = 64;

	if (listen(m_handle.GetFileDescriptor(), LISTEN_BACKLOG_SIZE) < 0)
	{
		throw std::system_error(errno, std::system_category(), "listen");
	}

	////////////////////////////////////////////////////////////////////////////////
}

TCPSocket TCPServerSocket::Accept()
{
	TCPSocket socket;

	socket.m_handle = Handle(accept(m_handle.GetFileDescriptor(), nullptr, nullptr));
	if (!socket.IsOpen())
	{
		throw std::system_error(errno, std::system_category(), "accept");
	}

	System::SetFileDescriptorNonBlocking(socket.GetHandle().GetFileDescriptor());
	System::SetFileDescriptorCloseOnExec(socket.GetHandle().GetFileDescriptor());

	return socket;
}
