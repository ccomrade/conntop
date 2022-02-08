#pragma once

#include <cstddef>
#include <cstdint>
#include <system_error>

#include "Base/IPAddress.h"

#include "Handle.h"

class TCPSocket
{
	Handle m_handle;

	friend class TCPServerSocket;

public:
	TCPSocket() = default;

	void StartConnect(const IPAddress& address, std::uint16_t port);
	void VerifyConnect();

	std::size_t Send(const void* buffer, std::size_t bufferSize);
	std::size_t Receive(void* buffer, std::size_t bufferSize);

	void Close()
	{
		m_handle.Close();
	}

	const Handle& GetHandle() const
	{
		return m_handle;
	}

	bool IsOpen() const
	{
		return m_handle.IsOpen();
	}
};
