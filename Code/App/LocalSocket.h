#pragma once

#include <cstdint>
#include <vector>

#include "Base/IPAddress.h"
#include "Base/IPProtocol.h"

struct LocalSocket
{
	std::uint64_t inode = 0;

	IPAddress localAddress;
	IPAddress remoteAddress;

	IPProtocol type = IPProtocol::UDP;

	std::uint16_t localPort = 0;
	std::uint16_t remotePort = 0;

	// Processes using this socket
	std::vector<std::uint32_t> pids;
};
