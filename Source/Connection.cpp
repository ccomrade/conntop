/**
 * @file
 * @brief Implementation of network connection classes.
 */

#include <map>
#include <stdexcept>

#include "Connection.hpp"

#include "rapidjson.hpp"
#include "rapidjson/document.h"

using rapidjson::Value;

namespace ctp
{
	static const std::map<KString, EConnectionType> g_connectionTypeMap = {
		{ "udp4", EConnectionType::UDP4 },
		{ "udp6", EConnectionType::UDP6 },
		{ "tcp4", EConnectionType::TCP4 },
		{ "tcp6", EConnectionType::TCP6 }
	};

	static const std::map<KString, EConnectionAction> g_connectionActionMap = {
		{ "CREATE", EConnectionAction::CREATE },
		{ "UPDATE", EConnectionAction::UPDATE },
		{ "REMOVE", EConnectionAction::REMOVE }
	};

	static const std::map<KString, UDP::EState> g_udpStateMap = {
		{ "LISTEN", UDP::LISTEN }
	};

	static const std::map<KString, TCP::EState> g_tcpStateMap = {
		{ "LISTEN",       TCP::LISTEN       },
		{ "SYN_SENT",     TCP::SYN_SENT     },
		{ "SYN_RECEIVED", TCP::SYN_RECEIVED },
		{ "ESTABLISHED",  TCP::ESTABLISHED  },
		{ "FIN_WAIT",     TCP::FIN_WAIT     },
		{ "FIN_WAIT_1",   TCP::FIN_WAIT_1   },
		{ "FIN_WAIT_2",   TCP::FIN_WAIT_2   },
		{ "CLOSE_WAIT",   TCP::CLOSE_WAIT   },
		{ "CLOSING",      TCP::CLOSING      },
		{ "LAST_ACK",     TCP::LAST_ACK     },
		{ "TIME_WAIT",    TCP::TIME_WAIT    },
		{ "CLOSED",       TCP::CLOSED       }
	};

	/**
	 * @brief Converts connection type to string.
	 * @param type Connection type.
	 * @return Connection type name.
	 */
	KString Connection::TypeToString(EConnectionType type)
	{
		switch (type)
		{
			case EConnectionType::UDP4: return "udp4";
			case EConnectionType::UDP6: return "udp6";
			case EConnectionType::TCP4: return "tcp4";
			case EConnectionType::TCP6: return "tcp6";
		}
		return "?";
	}

	/**
	 * @brief Converts connection action to string.
	 * @param action Connection action.
	 * @return Connection action name.
	 */
	KString Connection::ActionToString(EConnectionAction action)
	{
		switch (action)
		{
			case EConnectionAction::CREATE: return "CREATE";
			case EConnectionAction::UPDATE: return "UPDATE";
			case EConnectionAction::REMOVE: return "REMOVE";
		}
		return "?";
	}

	/**
	 * @brief Converts UDP connection state to string.
	 * @param state Connection state.
	 * @return Connection state name.
	 */
	KString UDP::StateToString(int state)
	{
		switch (static_cast<EState>(state))
		{
			case UNKNOWN: return "";  // UDP protocol is connectionless, so unknown state is valid state
			case LISTEN:  return "LISTEN";
		}
		return "?";
	}

	/**
	 * @brief Converts TCP connection state to string.
	 * @param state Connection state.
	 * @return Connection state name.
	 */
	KString TCP::StateToString(int state)
	{
		switch (static_cast<EState>(state))
		{
			case UNKNOWN:      break;
			case LISTEN:       return "LISTEN";
			case SYN_SENT:     return "SYN_SENT";
			case SYN_RECEIVED: return "SYN_RECEIVED";
			case ESTABLISHED:  return "ESTABLISHED";
			case FIN_WAIT:     return "FIN_WAIT";
			case FIN_WAIT_1:   return "FIN_WAIT_1";
			case FIN_WAIT_2:   return "FIN_WAIT_2";
			case CLOSE_WAIT:   return "CLOSE_WAIT";
			case CLOSING:      return "CLOSING";
			case LAST_ACK:     return "LAST_ACK";
			case TIME_WAIT:    return "TIME_WAIT";
			case CLOSED:       return "CLOSED";
		}
		return "?";
	}

	/**
	 * @brief Converts string to connection type.
	 * @param type Connection type name.
	 * @return Connection type.
	 * @throws std::invalid_argument If the string does not contain valid connection type.
	 */
	EConnectionType Connection::TypeToEnum(const KString & type)
	{
		auto it = g_connectionTypeMap.find(type);
		if (it != g_connectionTypeMap.end())
		{
			return it->second;
		}
		else
		{
			throw std::invalid_argument("Unknown connection type");
		}
	}

	/**
	 * @brief Converts string to connection action.
	 * @param action Connection action name.
	 * @return Connection action.
	 * @throws std::invalid_argument If the string does not contain valid connection action.
	 */
	EConnectionAction Connection::ActionToEnum(const KString & action)
	{
		auto it = g_connectionActionMap.find(action);
		if (it != g_connectionActionMap.end())
		{
			return it->second;
		}
		else
		{
			throw std::invalid_argument("Unknown connection action");
		}
	}

	/**
	 * @brief Converts string to UDP connection state.
	 * @param state Connection state name.
	 * @return Connection state.
	 */
	UDP::EState UDP::StateToEnum(const KString & state)
	{
		if (state.empty() || state == "?")
		{
			return UDP::UNKNOWN;
		}
		else
		{
			auto it = g_udpStateMap.find(state);
			return (it != g_udpStateMap.end()) ? it->second : UDP::UNKNOWN;
		}
	}

	/**
	 * @brief Converts string to TCP connection state.
	 * @param state Connection state name.
	 * @return Connection state.
	 */
	TCP::EState TCP::StateToEnum(const KString & state)
	{
		if (state.empty() || state == "?")
		{
			return TCP::UNKNOWN;
		}
		else
		{
			auto it = g_tcpStateMap.find(state);
			return (it != g_tcpStateMap.end()) ? it->second : TCP::UNKNOWN;
		}
	}

	/**
	 * @brief Converts connection state to string.
	 * @param state Connection state.
	 * @param type Connection type.
	 * @return Connection state name.
	 */
	KString Connection::StateToString(int state, EConnectionType type)
	{
		switch (type)
		{
			case EConnectionType::UDP4:
			case EConnectionType::UDP6:
			{
				return UDP::StateToString(state);
			}
			case EConnectionType::TCP4:
			case EConnectionType::TCP6:
			{
				return TCP::StateToString(state);
			}
		}
		return "?";
	}

	/**
	 * @brief Serializes connection.
	 * @param document JSON document that receives the serialized connection.
	 * @param action Connection action.
	 * @param updateFlags Connection update flags. Used only when action is update.
	 */
	void ConnectionData::serialize(rapidjson::Document & document, EConnectionAction action, int updateFlags) const
	{
		auto & allocator = document.GetAllocator();

		KString actionName = Connection::ActionToString(action);
		KString typeName = getTypeName();
		KString srcAddress = getSrcAddr().getNumericString();
		KString dstAddress = getDstAddr().getNumericString();

		document.AddMember("action", Value().SetString(actionName.c_str(), actionName.length()), allocator);
		document.AddMember("type", Value().SetString(typeName.c_str(), typeName.length()), allocator);
		document.AddMember("srcAddress", Value().SetString(srcAddress.c_str(), srcAddress.length()), allocator);
		document.AddMember("dstAddress", Value().SetString(dstAddress.c_str(), dstAddress.length()), allocator);

		if (hasPorts())
		{
			uint16_t srcPort = getSrcPort().getPortNumber();
			uint16_t dstPort = getDstPort().getPortNumber();

			document.AddMember("srcPort", Value().SetUint(srcPort), allocator);
			document.AddMember("dstPort", Value().SetUint(dstPort), allocator);
		}

		if (action == EConnectionAction::CREATE)
		{
			KString state = getStateName();

			document.AddMember("state", Value().SetString(state.c_str(), state.length()), allocator);

			const ConnectionTraffic & traffic = getTraffic();

			document.AddMember("rxPackets", Value().SetUint64(traffic.rxPackets), allocator);
			document.AddMember("txPackets", Value().SetUint64(traffic.txPackets), allocator);
			document.AddMember("rxBytes", Value().SetUint64(traffic.rxBytes), allocator);
			document.AddMember("txBytes", Value().SetUint64(traffic.txBytes), allocator);
			document.AddMember("rxSpeed", Value().SetUint64(traffic.rxSpeed), allocator);
			document.AddMember("txSpeed", Value().SetUint64(traffic.txSpeed), allocator);
		}
		else if (action == EConnectionAction::UPDATE)
		{
			document.AddMember("updateFlags", Value().SetInt(updateFlags), allocator);

			if (updateFlags & EConnectionUpdateFlags::PROTO_STATE)
			{
				KString state = getStateName();

				document.AddMember("state", Value().SetString(state.c_str(), state.length()), allocator);
			}

			const ConnectionTraffic & traffic = getTraffic();

			if (updateFlags & EConnectionUpdateFlags::RX_PACKETS)
			{
				document.AddMember("rxPackets", Value().SetUint64(traffic.rxPackets), allocator);
			}

			if (updateFlags & EConnectionUpdateFlags::TX_PACKETS)
			{
				document.AddMember("txPackets", Value().SetUint64(traffic.txPackets), allocator);
			}

			if (updateFlags & EConnectionUpdateFlags::RX_BYTES)
			{
				document.AddMember("rxBytes", Value().SetUint64(traffic.rxBytes), allocator);
			}

			if (updateFlags & EConnectionUpdateFlags::TX_BYTES)
			{
				document.AddMember("txBytes", Value().SetUint64(traffic.txBytes), allocator);
			}

			if (updateFlags & EConnectionUpdateFlags::RX_SPEED)
			{
				document.AddMember("rxSpeed", Value().SetUint64(traffic.rxSpeed), allocator);
			}

			if (updateFlags & EConnectionUpdateFlags::TX_SPEED)
			{
				document.AddMember("txSpeed", Value().SetUint64(traffic.txSpeed), allocator);
			}
		}
	}

	/**
	 * @brief Deserializes connection.
	 * @param document Serialized connection.
	 * @param callback Interface that receives the deserialized connection.
	 * @return True, if the connection was deserialized, otherwise false.
	 * @throws std::invalid_argument If the serialized connection is invalid.
	 */
	bool ConnectionData::Deserialize(const rapidjson::Value & document, IConnectionUpdateCallback *callback)
	{
		const auto actionIt = document.FindMember("action");
		if (actionIt == document.MemberEnd())
			throw std::invalid_argument("Missing connection action");
		else if (!actionIt->value.IsString())
			throw std::invalid_argument("Invalid connection action value type");

		const EConnectionAction action = Connection::ActionToEnum(actionIt->value.GetString());

		const auto connectionTypeIt = document.FindMember("type");
		if (connectionTypeIt == document.MemberEnd())
			throw std::invalid_argument("Missing connection type");
		else if (!connectionTypeIt->value.IsString())
			throw std::invalid_argument("Invalid connection type value type");

		const EConnectionType connectionType = Connection::TypeToEnum(connectionTypeIt->value.GetString());

		EPortType portType;
		switch (connectionType)
		{
			case EConnectionType::UDP4:
			case EConnectionType::UDP6:
			{
				portType = EPortType::UDP;
				break;
			}
			case EConnectionType::TCP4:
			case EConnectionType::TCP6:
			{
				portType = EPortType::TCP;
				break;
			}
			default:
			{
				return false;
			}
		}

		const auto srcAddressIt = document.FindMember("srcAddress");
		if (srcAddressIt == document.MemberEnd())
			throw std::invalid_argument("Missing source address");
		else if (!srcAddressIt->value.IsString())
			throw std::invalid_argument("Invalid source address value type");

		const auto dstAddressIt = document.FindMember("dstAddress");
		if (dstAddressIt == document.MemberEnd())
			throw std::invalid_argument("Missing destination address");
		else if (!dstAddressIt->value.IsString())
			throw std::invalid_argument("Invalid destination address value type");

		const auto srcPortIt = document.FindMember("srcPort");
		if (srcPortIt == document.MemberEnd())
			throw std::invalid_argument("Missing source port");
		else if (!srcPortIt->value.IsUint())
			throw std::invalid_argument("Invalid source port value type");

		const auto dstPortIt = document.FindMember("dstPort");
		if (dstPortIt == document.MemberEnd())
			throw std::invalid_argument("Missing destination port");
		else if (!dstPortIt->value.IsUint())
			throw std::invalid_argument("Invalid destination port value type");

		int updateFlags = 0;
		if (action == EConnectionAction::UPDATE)
		{
			const auto updateFlagsIt = document.FindMember("updateFlags");
			if (updateFlagsIt == document.MemberEnd())
				throw std::invalid_argument("Missing update flags");
			else if (!updateFlagsIt->value.IsInt())
				throw std::invalid_argument("Invalid update flags value type");

			updateFlags = updateFlagsIt->value.GetInt();
		}

		int state = 0;
		if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::PROTO_STATE)
		{
			const auto stateIt = document.FindMember("state");
			if (stateIt == document.MemberEnd())
				throw std::invalid_argument("Missing connection state");
			else if (!stateIt->value.IsString())
				throw std::invalid_argument("Invalid connection state value type");

			const KString stateString = stateIt->value.GetString();

			switch (portType)
			{
				case EPortType::UDP:
				{
					state = UDP::StateToEnum(stateString);
					break;
				}
				case EPortType::TCP:
				{
					state = TCP::StateToEnum(stateString);
					break;
				}
			}
		}

		ConnectionTraffic traffic;
		if (action != EConnectionAction::REMOVE)
		{
			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::RX_PACKETS)
			{
				const auto rxPacketsIt = document.FindMember("rxPackets");
				if (rxPacketsIt == document.MemberEnd())
					throw std::invalid_argument("Missing received packets count");
				else if (!rxPacketsIt->value.IsUint64())
					throw std::invalid_argument("Invalid received packets count value type");

				traffic.rxPackets = rxPacketsIt->value.GetUint64();
			}

			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::TX_PACKETS)
			{
				const auto txPacketsIt = document.FindMember("txPackets");
				if (txPacketsIt == document.MemberEnd())
					throw std::invalid_argument("Missing sent packets count");
				else if (!txPacketsIt->value.IsUint64())
					throw std::invalid_argument("Invalid sent packets count value type");

				traffic.txPackets = txPacketsIt->value.GetUint64();
			}

			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::RX_BYTES)
			{
				const auto rxBytesIt = document.FindMember("rxBytes");
				if (rxBytesIt == document.MemberEnd())
					throw std::invalid_argument("Missing received bytes count");
				else if (!rxBytesIt->value.IsUint64())
					throw std::invalid_argument("Invalid received bytes count value type");

				traffic.rxBytes = rxBytesIt->value.GetUint64();
			}

			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::TX_BYTES)
			{
				const auto txBytesIt = document.FindMember("txBytes");
				if (txBytesIt == document.MemberEnd())
					throw std::invalid_argument("Missing sent bytes count");
				else if (!txBytesIt->value.IsUint64())
					throw std::invalid_argument("Invalid sent bytes count value type");

				traffic.txBytes = txBytesIt->value.GetUint64();
			}

			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::RX_SPEED)
			{
				const auto rxSpeedIt = document.FindMember("rxSpeed");
				if (rxSpeedIt == document.MemberEnd())
					throw std::invalid_argument("Missing receive speed");
				else if (!rxSpeedIt->value.IsUint64())
					throw std::invalid_argument("Invalid receive speed value type");

				traffic.rxSpeed = rxSpeedIt->value.GetUint64();
			}

			if (action == EConnectionAction::CREATE || updateFlags & EConnectionUpdateFlags::TX_SPEED)
			{
				const auto txSpeedIt = document.FindMember("txSpeed");
				if (txSpeedIt == document.MemberEnd())
					throw std::invalid_argument("Missing send speed");
				else if (!txSpeedIt->value.IsUint64())
					throw std::invalid_argument("Invalid send speed value type");

				traffic.txSpeed = txSpeedIt->value.GetUint64();
			}
		}

		const bool add = (action == EConnectionAction::CREATE);

		const KString srcAddressString = srcAddressIt->value.GetString();
		const KString dstAddressString = dstAddressIt->value.GetString();

		const uint16_t srcPortNumber = srcPortIt->value.GetUint();
		const uint16_t dstPortNumber = dstPortIt->value.GetUint();

		AddressData *srcAddress = nullptr;
		AddressData *dstAddress = nullptr;

		switch (connectionType)
		{
			case EConnectionType::UDP4:
			case EConnectionType::TCP4:
			{
				srcAddress = callback->getAddress(AddressIP4::CreateFromString(srcAddressString), add);
				dstAddress = callback->getAddress(AddressIP4::CreateFromString(dstAddressString), add);
				break;
			}
			case EConnectionType::UDP6:
			case EConnectionType::TCP6:
			{
				srcAddress = callback->getAddress(AddressIP6::CreateFromString(srcAddressString), add);
				dstAddress = callback->getAddress(AddressIP6::CreateFromString(dstAddressString), add);
				break;
			}
		}

		if (!srcAddress || !dstAddress)
		{
			return false;
		}

		PortData *srcPort = callback->getPort(Port(portType, srcPortNumber), add);
		PortData *dstPort = callback->getPort(Port(portType, dstPortNumber), add);

		if (!srcPort || !dstPort)
		{
			return false;
		}

		Connection connection(*srcAddress, *srcPort, *dstAddress, *dstPort);

		switch (action)
		{
			case EConnectionAction::CREATE:
			{
				callback->add(connection, traffic, state);

				break;
			}
			case EConnectionAction::UPDATE:
			{
				ConnectionData *pData = callback->find(connection);
				if (!pData)
				{
					return false;
				}

				int flags = 0;

				if (updateFlags & EConnectionUpdateFlags::PROTO_STATE)
				{
					pData->setState(state);
					flags |= EConnectionUpdateFlags::PROTO_STATE;
				}

				ConnectionTraffic & connectionTraffic = pData->getTraffic();

				if (updateFlags & EConnectionUpdateFlags::RX_PACKETS)
				{
					connectionTraffic.rxPackets = traffic.rxPackets;
					flags |= EConnectionUpdateFlags::RX_PACKETS;
				}

				if (updateFlags & EConnectionUpdateFlags::TX_PACKETS)
				{
					connectionTraffic.txPackets = traffic.txPackets;
					flags |= EConnectionUpdateFlags::TX_PACKETS;
				}

				if (updateFlags & EConnectionUpdateFlags::RX_BYTES)
				{
					connectionTraffic.rxBytes = traffic.rxBytes;
					flags |= EConnectionUpdateFlags::RX_BYTES;
				}

				if (updateFlags & EConnectionUpdateFlags::TX_BYTES)
				{
					connectionTraffic.txBytes = traffic.txBytes;
					flags |= EConnectionUpdateFlags::TX_BYTES;
				}

				if (updateFlags & EConnectionUpdateFlags::RX_SPEED)
				{
					connectionTraffic.rxSpeed = traffic.rxSpeed;
					flags |= EConnectionUpdateFlags::RX_SPEED;
				}

				if (updateFlags & EConnectionUpdateFlags::TX_SPEED)
				{
					connectionTraffic.txSpeed = traffic.txSpeed;
					flags |= EConnectionUpdateFlags::TX_SPEED;
				}

				if (flags)
				{
					callback->update(*pData, flags);
				}

				break;
			}
			case EConnectionAction::REMOVE:
			{
				callback->remove(connection);

				break;
			}
		}

		return true;
	}
}
