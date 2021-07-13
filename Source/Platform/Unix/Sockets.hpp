/**
 * @file
 * @brief Socket classes for Unix platform.
 */

#pragma once

#include <string>
#include <memory>
#include <exception>

#include "Types.hpp"
#include "KString.hpp"
#include "Address.hpp"
#include "Port.hpp"
#include "Util.hpp"

namespace ctp
{
	enum struct EStreamSocketType
	{
		TCP = 6
	};

	class SocketEndpoint
	{
		std::unique_ptr<IAddress> m_pAddress;
		std::unique_ptr<Port> m_pPort;

	public:
		SocketEndpoint()
		: m_pAddress(),
		  m_pPort()
		{
		}

		SocketEndpoint(std::unique_ptr<IAddress> && pAddress, std::unique_ptr<Port> && pPort)
		: m_pAddress(std::move(pAddress)),
		  m_pPort(std::move(pPort))
		{
		}

		bool isEmpty() const
		{
			return (hasAddress() && hasPort()) ? false : true;
		}

		bool hasAddress() const
		{
			return m_pAddress.get() != nullptr;
		}

		bool hasPort() const
		{
			return m_pPort.get() != nullptr;
		}

		const IAddress & getAddress() const
		{
			return *m_pAddress;
		}

		const Port & getPort() const
		{
			return *m_pPort;
		}

		std::string toString() const
		{
			if (hasAddress() && hasPort())
			{
				return Util::AddressPortToString(*m_pAddress, *m_pPort);
			}

			return std::string();
		}
	};

	class SocketException : public std::exception
	{
		int m_errorNumber;
		std::string m_errorString;

	public:
		SocketException(int errorNumber)
		: m_errorNumber(errorNumber),
		  m_errorString(Util::ErrnoToString(errorNumber))
		{
		}

		SocketException(int errorNumber, std::string errorString)
		: m_errorNumber(errorNumber),
		  m_errorString(std::move(errorString))
		{
		}

		const char *what() const noexcept override
		{
			return m_errorString.c_str();
		}

		const std::string & getString() const
		{
			return m_errorString;
		}

		int getErrorNumber() const
		{
			return m_errorNumber;
		}
	};

	/**
	 * @brief Fully asynchronous network stream client socket.
	 */
	class StreamSocket
	{
		int m_fd;
		EStreamSocketType m_type;

		StreamSocket(int fd, EStreamSocketType type)
		: m_fd(fd),
		  m_type(type)
		{
		}

		friend class StreamServerSocket;

	public:
		StreamSocket()
		: m_fd(-1),
		  m_type()
		{
		}

		// no copy
		StreamSocket(const StreamSocket &) = delete;
		StreamSocket & operator=(const StreamSocket &) = delete;

		StreamSocket(StreamSocket && other)
		: m_fd(other.m_fd),
		  m_type(other.m_type)
		{
			other.m_fd = -1;
		}

		StreamSocket & operator=(StreamSocket && other)
		{
			if (this != &other)
			{
				close_nothrow();
				m_fd = other.m_fd;
				m_type = other.m_type;
				other.m_fd = -1;
			}
			return *this;
		}

		~StreamSocket()
		{
			close_nothrow();
		}

		bool isConnected() const
		{
			return m_fd >= 0;
		}

		void connect(const IAddress & address, uint16_t port, EStreamSocketType type);

		void verifyConnect();

		void close();
		bool close_nothrow();

		size_t send(const char *data, size_t dataLength);
		size_t receive(char *buffer, size_t bufferSize);

		int getType() const;
		KString getTypeName() const;

		SocketEndpoint getLocalEndpoint() const;
		SocketEndpoint getRemoteEndpoint() const;

		// Platform-specific functions

		int getFD()
		{
			return m_fd;
		}
	};

	/**
	 * @brief Fully asynchronous network stream server socket.
	 */
	class StreamServerSocket
	{
		int m_fd;
		EStreamSocketType m_type;

	public:
		StreamServerSocket()
		: m_fd(-1),
		  m_type()
		{
		}

		// no copy
		StreamServerSocket(const StreamServerSocket &) = delete;
		StreamServerSocket & operator=(const StreamServerSocket &) = delete;

		StreamServerSocket(StreamServerSocket && other)
		: m_fd(other.m_fd),
		  m_type(other.m_type)
		{
			other.m_fd = -1;
		}

		StreamServerSocket & operator=(StreamServerSocket && other)
		{
			if (this != &other)
			{
				close_nothrow();
				m_fd = other.m_fd;
				m_type = other.m_type;
				other.m_fd = -1;
			}
			return *this;
		}

		~StreamServerSocket()
		{
			close_nothrow();
		}

		bool isOpen() const
		{
			return m_fd >= 0;
		}

		void open(const IAddress & address, uint16_t port, EStreamSocketType type);
		void open(const IAddress & address, const KString & portString, EStreamSocketType type);
		void open(const KString & addressString, uint16_t port, EStreamSocketType type);
		void open(const KString & addressString, const KString & portString, EStreamSocketType type);
		void openAny(EAddressType addressType, uint16_t port, EStreamSocketType type);
		void openAny(EAddressType addressType, const KString & portString, EStreamSocketType type);
		void openLocalhost(EAddressType addressType, uint16_t port, EStreamSocketType type);
		void openLocalhost(EAddressType addressType, const KString & portString, EStreamSocketType type);

		void close();
		bool close_nothrow();

		StreamSocket accept();

		int getType() const;
		KString getTypeName() const;

		SocketEndpoint getListenEndpoint() const;

		// Platform-specific functions

		int getFD()
		{
			return m_fd;
		}
	};
}
