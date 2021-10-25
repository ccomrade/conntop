#pragma once

#include <stdint.h>

#include "Base/IPAddress.h"
#include "Base/IPProtocol.h"

struct Connection
{
	IPAddress srcAddress;
	IPAddress dstAddress;

	IPProtocol type = IPProtocol::UDP;

	// TODO: union

	int state = 0;

	uint16_t srcPort = 0;
	uint16_t dstPort = 0;

	uint8_t icmpType = 0;
	uint8_t icmpCode = 0;

	uint64_t rxPacketCount = 0;
	uint64_t rxPacketSpeed = 0;
	uint64_t txPacketCount = 0;
	uint64_t txPacketSpeed = 0;

	uint64_t rxByteCount = 0;
	uint64_t rxByteSpeed = 0;
	uint64_t txByteCount = 0;
	uint64_t txByteSpeed = 0;
};
