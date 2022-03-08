#pragma once

#include <cstdint>

#include "Base/IPAddress.h"

#include "Handle.h"
#include "TCPSocket.h"

class TCPServerSocket
{
	Handle m_handle;

public:
	TCPServerSocket() = default;

	void Open(const IPAddress& address, std::uint16_t port);

	TCPSocket Accept();

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
