#include <cerrno>
#include <system_error>

#include "Base/Endian.h"
#include "System/Log.h"
#include "System/System.h"

#include "Conntrack.h"

Conntrack::Conntrack()
{
	m_handle = Handle(nfct_open(CONNTRACK, 0));
	if (!m_handle)
	{
		throw std::system_error(errno, std::system_category(), "Failed to open conntrack");
	}

	const int fd = nfct_fd(m_handle.get());

	// only set close-on-exec and keep the Netlink socket blocking to ensure that nfct_query works as expected
	System::SetFileDescriptorCloseOnExec(fd);

	if (nfct_callback_register(m_handle.get(), NFCT_T_ALL, UpdateCallbackWrapper, this) < 0)
	{
		throw std::system_error(errno, std::system_category(), "Failed to register conntrack callback");
	}

	LOG_DEBUG(Format("[Conntrack] Opened on file descriptor %d", fd));
}

Conntrack::~Conntrack()
{
	const int fd = nfct_fd(m_handle.get());

	LOG_DEBUG(Format("[Conntrack] Closed on file descriptor %d", fd));
}

void Conntrack::Update()
{
	m_connections.clear();

	// both IPv4 and IPv6
	const std::uint32_t addressFamily = AF_UNSPEC;

	// dump content of the conntrack table
	// calls UpdateCallback for each entry
	if (nfct_query(m_handle.get(), NFCT_Q_DUMP, &addressFamily) < 0)
	{
		throw std::system_error(errno, std::system_category(), "Failed to query conntrack");
	}
}

void Conntrack::UpdateCallback(nf_conntrack_msg_type type, nf_conntrack* ct)
{
	if (type == NFCT_T_UPDATE)
	{
		m_connections.emplace_back();

		// parse connection entry
		const bool success = ToConnection(ct, m_connections.back());

		if (!success)
		{
			// skip unsupported connection types
			m_connections.pop_back();
		}
	}
	else
	{
		LOG_DEBUG(Format("[Conntrack] Unexpected message of type %d", type));
	}
}

int Conntrack::UpdateCallbackWrapper(nf_conntrack_msg_type type, nf_conntrack* ct, void* data)
{
	static_cast<Conntrack*>(data)->UpdateCallback(type, ct);

	// always continue to ensure that receive buffer of the Netlink socket is empty
	return NFCT_CB_CONTINUE;
}

bool Conntrack::ToConnection(nf_conntrack* ct, Connection& result)
{
	const int addressFamily = nfct_get_attr_u8(ct, ATTR_L3PROTO);

	switch (addressFamily)
	{
		case AF_INET:
		{
			result.srcAddress.CopyFrom(nfct_get_attr(ct, ATTR_IPV4_SRC), IPAddressType::IPv4);
			result.dstAddress.CopyFrom(nfct_get_attr(ct, ATTR_IPV4_DST), IPAddressType::IPv4);
			break;
		}
		case AF_INET6:
		{
			result.srcAddress.CopyFrom(nfct_get_attr(ct, ATTR_IPV6_SRC), IPAddressType::IPv6);
			result.dstAddress.CopyFrom(nfct_get_attr(ct, ATTR_IPV6_DST), IPAddressType::IPv6);
			break;
		}
		default:
		{
			LOG_DEBUG(Format("[Conntrack] Unknown address family %d", addressFamily));
			return false;
		}
	}

	const int protocol = nfct_get_attr_u8(ct, ATTR_L4PROTO);

	switch (protocol)
	{
		case IPPROTO_UDP:
		{
			result.type = IPProtocol::UDP;
			result.state = 0;
			break;
		}
		case IPPROTO_TCP:
		{
			result.type = IPProtocol::TCP;
			result.state = nfct_get_attr_u8(ct, ATTR_TCP_STATE);
			break;
		}
		case IPPROTO_SCTP:
		{
			result.type = IPProtocol::SCTP;
			result.state = nfct_get_attr_u8(ct, ATTR_SCTP_STATE);
			break;
		}
		case IPPROTO_DCCP:
		{
			result.type = IPProtocol::DCCP;
			result.state = nfct_get_attr_u8(ct, ATTR_DCCP_STATE);
			break;
		}
		case IPPROTO_ICMP:
		{
			result.type = IPProtocol::ICMP;
			result.state = 0;
			break;
		}
		case IPPROTO_ICMPV6:
		{
			result.type = IPProtocol::ICMPv6;
			result.state = 0;
			break;
		}
		default:
		{
			LOG_DEBUG(Format("[Conntrack] Unknown protocol %d", protocol));
			return false;
		}
	}

	switch (result.type)
	{
		case IPProtocol::UDP:
		case IPProtocol::TCP:
		case IPProtocol::SCTP:
		case IPProtocol::DCCP:
		{
			result.srcPort = BigEndian(nfct_get_attr_u16(ct, ATTR_PORT_SRC));
			result.dstPort = BigEndian(nfct_get_attr_u16(ct, ATTR_PORT_DST));
			break;
		}
		case IPProtocol::ICMP:
		case IPProtocol::ICMPv6:
		{
			result.icmpType = nfct_get_attr_u8(ct, ATTR_ICMP_TYPE);
			result.icmpCode = nfct_get_attr_u8(ct, ATTR_ICMP_CODE);
			break;
		}
	}

	result.rxPacketCount = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_PACKETS);
	result.txPacketCount = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_PACKETS);
	result.rxByteCount = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);
	result.txByteCount = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);

	return true;
}
