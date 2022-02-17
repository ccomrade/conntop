#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cerrno>

#include "TCPSocket.h"
#include "System.h"

void TCPSocket::StartConnect(const IPAddress& address, std::uint16_t port)
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
		sockaddr_storage socketAddress = {};

		switch (address.type)
		{
			case IPAddressType::IPv4:
			{
				auto sa4 = reinterpret_cast<sockaddr_in*>(&socketAddress);

				sa4->sin_family = AF_INET;
				sa4->sin_port = htons(port);
				address.CopyTo(&sa4->sin_addr);

				break;
			}
			case IPAddressType::IPv6:
			{
				auto sa6 = reinterpret_cast<sockaddr_in6*>(&socketAddress);

				sa6->sin6_family = AF_INET6;
				sa6->sin6_port = htons(port);
				address.CopyTo(&sa6->sin6_addr);

				break;
			}
		}

		return socketAddress;
	}();

	////////////////////////////////////////////////////////////////////////////////

	if (connect(m_handle.GetFileDescriptor(), reinterpret_cast<const sockaddr*>(&socketAddress), sizeof socketAddress) < 0)
	{
		const int errorNumber = errno;

		if (errorNumber != EINPROGRESS)
		{
			throw std::system_error(errorNumber, std::system_category(), "connect");
		}
	}

	////////////////////////////////////////////////////////////////////////////////
}

void TCPSocket::VerifyConnect()
{
	int errorNumber = 0;
	socklen_t errorNumberSize = sizeof errorNumber;

	if (getsockopt(m_handle.GetFileDescriptor(), SOL_SOCKET, SO_ERROR, &errorNumber, &errorNumberSize) < 0)
	{
		throw std::system_error(errno, std::system_category(), "getsockopt");
	}

	if (errorNumber)
	{
		throw std::system_error(errorNumber, std::system_category(), "connect");
	}
}

std::size_t TCPSocket::Send(const void* buffer, std::size_t bufferSize)
{
	const ssize_t bytesSent = send(m_handle.GetFileDescriptor(), buffer, bufferSize, 0);

	if (bytesSent < 0)
	{
		throw std::system_error(errno, std::system_category(), "send");
	}
	else
	{
		return bytesSent;
	}
}

std::size_t TCPSocket::Receive(void* buffer, std::size_t bufferSize)
{
	const ssize_t bytesReceived = recv(m_handle.GetFileDescriptor(), buffer, bufferSize, 0);

	if (bytesReceived < 0)
	{
		throw std::system_error(errno, std::system_category(), "recv");
	}
	else
	{
		return bytesReceived;
	}
}
