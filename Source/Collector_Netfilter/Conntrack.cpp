/**
 * @file
 * @brief Implementation of Conntrack class.
 */

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cerrno>
#include <system_error>

#include "Conntrack.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "Exception.hpp"
#include "Util.hpp"

namespace ctp
{
	static int TCPStateToEnum(uint8_t rawState)
	{
		switch (rawState)
		{
			case TCP_CONNTRACK_NONE:        return TCP::UNKNOWN;
			case TCP_CONNTRACK_SYN_SENT:    return TCP::SYN_SENT;
			case TCP_CONNTRACK_SYN_SENT2:   return TCP::SYN_SENT;  // TCP simultaneous open
			case TCP_CONNTRACK_SYN_RECV:    return TCP::SYN_RECEIVED;
			case TCP_CONNTRACK_ESTABLISHED: return TCP::ESTABLISHED;
			case TCP_CONNTRACK_FIN_WAIT:    return TCP::FIN_WAIT;
			case TCP_CONNTRACK_CLOSE_WAIT:  return TCP::CLOSE_WAIT;
			case TCP_CONNTRACK_LAST_ACK:    return TCP::LAST_ACK;
			case TCP_CONNTRACK_TIME_WAIT:   return TCP::TIME_WAIT;
			case TCP_CONNTRACK_CLOSE:       return TCP::CLOSING;
		}
		return TCP::UNKNOWN;
	}

	static bool TCPStateIsEqual(int state, uint8_t rawState)
	{
		switch (rawState)
		{
			case TCP_CONNTRACK_NONE:
			{
				if (state == TCP::UNKNOWN)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_SYN_SENT:
			case TCP_CONNTRACK_SYN_SENT2:  // TCP simultaneous open
			{
				if (state == TCP::SYN_SENT)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_SYN_RECV:
			{
				if (state == TCP::SYN_RECEIVED)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_ESTABLISHED:
			{
				if (state == TCP::ESTABLISHED)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_FIN_WAIT:
			{
				if (state == TCP::FIN_WAIT)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_CLOSE_WAIT:
			{
				if (state == TCP::CLOSE_WAIT)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_LAST_ACK:
			{
				if (state == TCP::LAST_ACK)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_TIME_WAIT:
			{
				if (state == TCP::TIME_WAIT)
				{
					return true;
				}
				break;
			}
			case TCP_CONNTRACK_CLOSE:
			{
				if (state == TCP::CLOSING)
				{
					return true;
				}
				break;
			}
		}
		return false;
	}

	static bool ExtractAddressPort(AddressData **pSrcAddress, PortData **pSrcPort,
	                                AddressData **pDstAddress, PortData **pDstPort,
	                                nf_conntrack *ct, IConnectionUpdateCallback *callback, bool add)
	{
		EAddressType addressType;
		switch (nfct_get_attr_u8(ct, ATTR_L3PROTO))
		{
			case AF_INET:
			{
				addressType = EAddressType::IP4;
				break;
			}
			case AF_INET6:
			{
				addressType = EAddressType::IP6;
				break;
			}
			default:
			{
				return false;
			}
		}

		EPortType portType;
		switch (nfct_get_attr_u8(ct, ATTR_L4PROTO))
		{
			case IPPROTO_UDP:
			{
				portType = EPortType::UDP;
				break;
			}
			case IPPROTO_TCP:
			{
				portType = EPortType::TCP;
				break;
			}
			default:
			{
				return false;
			}
		}

		AddressData *srcAddress = nullptr;
		AddressData *dstAddress = nullptr;

		switch (addressType)
		{
			case EAddressType::IP4:
			{
				uint32_t srcAddrRaw = nfct_get_attr_u32(ct, ATTR_IPV4_SRC);
				uint32_t dstAddrRaw = nfct_get_attr_u32(ct, ATTR_IPV4_DST);
				srcAddress = callback->getAddress(AddressIP4(srcAddrRaw), add);
				dstAddress = callback->getAddress(AddressIP4(dstAddrRaw), add);
				break;
			}
			case EAddressType::IP6:
			{
				const void *pSrcAddrRaw = nfct_get_attr(ct, ATTR_IPV6_SRC);
				const void *pDstAddrRaw = nfct_get_attr(ct, ATTR_IPV6_DST);
				const AddressIP6::RawAddr & srcAddrRaw = *static_cast<const AddressIP6::RawAddr*>(pSrcAddrRaw);
				const AddressIP6::RawAddr & dstAddrRaw = *static_cast<const AddressIP6::RawAddr*>(pDstAddrRaw);
				srcAddress = callback->getAddress(AddressIP6(srcAddrRaw), add);
				dstAddress = callback->getAddress(AddressIP6(dstAddrRaw), add);
				break;
			}
		}

		if (!srcAddress || !dstAddress)
		{
			return false;
		}

		const uint16_t srcPortNumber = ntohs(nfct_get_attr_u16(ct, ATTR_PORT_SRC));
		const uint16_t dstPortNumber = ntohs(nfct_get_attr_u16(ct, ATTR_PORT_DST));

		PortData *srcPort = callback->getPort(Port(portType, srcPortNumber), add);
		PortData *dstPort = callback->getPort(Port(portType, dstPortNumber), add);

		if (!srcPort || !dstPort)
		{
			return false;
		}

		(*pSrcAddress) = srcAddress;
		(*pDstAddress) = dstAddress;
		(*pSrcPort) = srcPort;
		(*pDstPort) = dstPort;

		return true;
	}

	ConntrackSocket::ConntrackSocket(unsigned int events)
	{
		m_socket = nfct_open(CONNTRACK, events);
		if (!m_socket)
		{
			std::string errMsg = "Unable to open conntrack";
			if (events != 0)
			{
				errMsg += " event";
			}
			errMsg += " socket: ";
			errMsg += Util::ErrnoToString();
			throw Exception(std::move(errMsg), "Collector_Netfilter");
		}

		const int fd = getFD();

		if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		{
			const int err = errno;
			nfct_close(m_socket);
			throw std::system_error(err, std::system_category(), "Unable to set conntrack socket non-blocking");
		}

		if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
		{
			const int err = errno;
			nfct_close(m_socket);
			throw std::system_error(err, std::system_category(), "Unable to set conntrack socket close-on-exec");
		}

		gLog->info("[Collector_Netfilter] Created conntrack socket on %d (events: 0x%X)", fd, events);
	}

	ConntrackSocket::~ConntrackSocket()
	{
		const int fd = getFD();

		nfct_close(m_socket);

		gLog->info("[Collector_Netfilter] Closed conntrack socket on %d", fd);
	}

	Conntrack::Conntrack()
	: m_eventQueue(),
	  m_querySocket(),
	  m_eventSocket(NF_NETLINK_CONNTRACK_NEW | NF_NETLINK_CONNTRACK_DESTROY),
	  m_callback(),
	  m_isInitialized(false),
	  m_isRefillRequired(true),
	  m_isPaused()
	{
		if (nfct_callback_register(m_querySocket.get(), NFCT_T_ALL, QueryCallback, this) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to register conntrack query callback");
		}

		if (nfct_callback_register(m_eventSocket.get(), NFCT_T_ALL, EventCallback, this) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to register conntrack event callback");
		}

		gApp->getPollSystem()->add(m_eventSocket, EPollFlags::INPUT, EventPollHandler, this);
	}

	Conntrack::~Conntrack()
	{
		gApp->getPollSystem()->remove(m_eventSocket);
	}

	void Conntrack::init(IConnectionUpdateCallback *callback)
	{
		if (!callback)
		{
			return;
		}

		m_callback = callback;
		m_isInitialized = true;
		m_isRefillRequired = true;
		m_isPaused = false;
	}

	void Conntrack::onUpdate()
	{
		if (!m_isInitialized || m_isPaused)
		{
			return;
		}

		if (m_isRefillRequired)
		{
			m_callback->clear();
			// connection list is now empty and it needs to be refilled
		}
		else
		{
			// process conntrack events received from kernel since last update
			for (const ConntrackEvent & event : m_eventQueue)
			{
				switch (event.getType())
				{
					case ConntrackEvent::NEW_CONNECTION:
					{
						m_callback->add(event.getConnection());
						break;
					}
					case ConntrackEvent::REMOVED_CONNECTION:
					{
						m_callback->remove(event.getConnection());
						break;
					}
				}
			}
		}

		m_eventQueue.clear();

		const uint32_t addressFamily = AF_UNSPEC;
		// dump conntrack table to either refill connection list or update traffic and state of connections in the list
		if (nfct_query(m_querySocket.get(), NFCT_Q_DUMP, &addressFamily) < 0)  // calls QueryCallback
		{
			std::string errMsg = "Unable to dump content of conntrack table: ";
			errMsg += Util::ErrnoToString();
			throw Exception(std::move(errMsg), "Collector_Netfilter");
		}

		if (m_isRefillRequired)
		{
			gLog->debug("[Collector_Netfilter] Refill done");
		}

		m_isRefillRequired = false;
	}

	void Conntrack::handleQuery(nf_conntrack *ct)
	{
		AddressData *srcAddress, *dstAddress;
		PortData *srcPort, *dstPort;
		if (!ExtractAddressPort(&srcAddress, &srcPort, &dstAddress, &dstPort, ct, m_callback, m_isRefillRequired))
		{
			return;
		}

		Connection connection(*srcAddress, *srcPort, *dstAddress, *dstPort);

		if (m_isRefillRequired)
		{
			ConnectionTraffic traffic;
			traffic.rxPackets = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_PACKETS);
			traffic.txPackets = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_PACKETS);
			traffic.rxBytes = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);
			traffic.txBytes = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);

			int state = 0;

			switch (connection.getType())
			{
				case EConnectionType::TCP4:
				case EConnectionType::TCP6:
				{
					const uint8_t rawState = nfct_get_attr_u8(ct, ATTR_TCP_STATE);
					state = TCPStateToEnum(rawState);
					break;
				}
				default:
				{
					break;
				}
			}

			m_callback->add(connection, traffic, state);
		}
		else
		{
			ConnectionData *pData = m_callback->find(connection);
			if (!pData)
			{
				return;
			}

			int updateFlags = 0;

			switch (connection.getType())
			{
				case EConnectionType::TCP4:
				case EConnectionType::TCP6:
				{
					const uint8_t rawState = nfct_get_attr_u8(ct, ATTR_TCP_STATE);
					if (!TCPStateIsEqual(pData->getState(), rawState))
					{
						pData->setState(TCPStateToEnum(rawState));
						updateFlags |= EConnectionUpdateFlags::PROTO_STATE;
					}
					break;
				}
				default:
				{
					break;
				}
			}

			uint64_t value;
			uint64_t speed;

			ConnectionTraffic & traffic = pData->getTraffic();

			// number of received packets
			value = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_PACKETS);
			if (traffic.rxPackets != value)
			{
				traffic.rxPackets = value;
				updateFlags |= EConnectionUpdateFlags::RX_PACKETS;
			}

			// number of sent packets
			value = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_PACKETS);
			if (traffic.txPackets != value)
			{
				traffic.txPackets = value;
				updateFlags |= EConnectionUpdateFlags::TX_PACKETS;
			}

			// number of received bytes and receive speed
			value = nfct_get_attr_u64(ct, ATTR_REPL_COUNTER_BYTES);
			speed = value - traffic.rxBytes;
			if (speed != 0)
			{
				traffic.rxBytes = value;
				updateFlags |= EConnectionUpdateFlags::RX_BYTES;
				if (speed != traffic.rxSpeed)
				{
					traffic.rxSpeed = speed;
					updateFlags |= EConnectionUpdateFlags::RX_SPEED;
				}
			}
			else if (traffic.rxSpeed != 0)
			{
				traffic.rxSpeed = 0;
				updateFlags |= EConnectionUpdateFlags::RX_SPEED;
			}

			// number of sent bytes and send speed
			value = nfct_get_attr_u64(ct, ATTR_ORIG_COUNTER_BYTES);
			speed = value - traffic.txBytes;
			if (speed != 0)
			{
				traffic.txBytes = value;
				updateFlags |= EConnectionUpdateFlags::TX_BYTES;
				if (speed != traffic.txSpeed)
				{
					traffic.txSpeed = speed;
					updateFlags |= EConnectionUpdateFlags::TX_SPEED;
				}
			}
			else if (traffic.txSpeed != 0)
			{
				traffic.txSpeed = 0;
				updateFlags |= EConnectionUpdateFlags::TX_SPEED;
			}

			// update the connection if some value has changed
			if (updateFlags)
			{
				m_callback->update(*pData, updateFlags);
			}
		}
	}

	void Conntrack::handleEvent(nf_conntrack *ct, ConntrackEvent::EType eventType)
	{
		const bool isNew = (eventType == ConntrackEvent::NEW_CONNECTION);

		AddressData *srcAddress, *dstAddress;
		PortData *srcPort, *dstPort;
		if (!ExtractAddressPort(&srcAddress, &srcPort, &dstAddress, &dstPort, ct, m_callback, isNew))
		{
			return;
		}

		Connection connection(*srcAddress, *srcPort, *dstAddress, *dstPort);

		m_eventQueue.emplace_back(eventType, connection);
	}

	int Conntrack::QueryCallback(nf_conntrack_msg_type /* unused */, nf_conntrack *ct, void *param)
	{
		Conntrack *self = static_cast<Conntrack*>(param);

		self->handleQuery(ct);

		return NFCT_CB_CONTINUE;
	}

	int Conntrack::EventCallback(nf_conntrack_msg_type type, nf_conntrack *ct, void *param)
	{
		Conntrack *self = static_cast<Conntrack*>(param);

		if (!self->m_isRefillRequired)
		{
			switch (type)
			{
				case NFCT_T_NEW:
				{
					self->handleEvent(ct, ConntrackEvent::NEW_CONNECTION);
					break;
				}
				case NFCT_T_DESTROY:
				{
					self->handleEvent(ct, ConntrackEvent::REMOVED_CONNECTION);
					break;
				}
				default:
				{
					break;
				}
			}
		}

		return NFCT_CB_CONTINUE;
	}

	void Conntrack::EventPollHandler(int flags, void *param)
	{
		Conntrack *self = static_cast<Conntrack*>(param);

		if (flags & EPollFlags::INPUT)
		{
			if (nfct_catch(self->m_eventSocket.get()) < 0)  // calls EventCallback
			{
				if (errno == ENOBUFS)
				{
					gLog->warning("[Collector_Netfilter] Conntrack event socket buffer is full");
					self->m_isRefillRequired = true;
				}
				else if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					// all events were received from kernel
				}
				else
				{
					std::string errMsg = "Unable to receive conntrack events: ";
					errMsg += Util::ErrnoToString();
					throw Exception(std::move(errMsg), "Collector_Netfilter");
				}
			}
		}

		if (flags & EPollFlags::ERROR)
		{
			throw Exception("Conntrack event socket poll failed", "Collector_Netfilter");
		}

		gApp->getPollSystem()->reset(self->m_eventSocket, EPollFlags::INPUT);
	}
}
