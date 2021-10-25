#pragma once

#include <stdint.h>
#include <vector>

#include "Base/IPAddress.h"
#include "Base/IPProtocol.h"

struct LocalSocket
{
	uint64_t inode = 0;

	IPAddress localAddress;
	IPAddress remoteAddress;

	IPProtocol type = IPProtocol::UDP;

	uint16_t localPort = 0;
	uint16_t remotePort = 0;

	// Processes using this socket
	std::vector<uint32_t> pids;
};
