/**
 * @file
 * @brief Network connection classes.
 */

#pragma once

#include "Types.hpp"
#include "KString.hpp"
#include "Address.hpp"
#include "Port.hpp"
#include "Hash.hpp"

#include "rapidjson.hpp"

namespace ctp
{
	/**
	 * @brief Network connection type.
	 */
	enum struct EConnectionType
	{
		UDP4,  //!< UDP over IPv4
		UDP6,  //!< UDP over IPv6
		TCP4,  //!< TCP over IPv4
		TCP6   //!< TCP over IPv6
	};

	/**
	 * @brief Network connection action type.
	 * Used for connection serialization.
	 */
	enum struct EConnectionAction
	{
		CREATE,
		UPDATE,
		REMOVE
	};

	/**
	 * @brief Network connection sort mode.
	 */
	enum struct EConnectionSortMode
	{
		NONE,          //!< No sorting is performed.
		PROTO,         //!< Sort by connection protocol.
		PROTO_STATE,   //!< Sort by connection protocol state.
		SRC_ADDRESS,   //!< Sort by source address.
		SRC_HOSTNAME,  //!< Sort by hostname of source address.
		SRC_ASN,       //!< Sort by AS number of source address.
		SRC_COUNTRY,   //!< Sort by country code of source address.
		SRC_PORT,      //!< Sort by source port number.
		SRC_SERVICE,   //!< Sort by service name of source port.
		DST_ADDRESS,   //!< Sort by destination address.
		DST_HOSTNAME,  //!< Sort by hostname of destination address.
		DST_ASN,       //!< Sort by AS number of destination address.
		DST_COUNTRY,   //!< Sort by country code of destination address.
		DST_PORT,      //!< Sort by destination port number.
		DST_SERVICE,   //!< Sort by service name of destination port.
		RX_PACKETS,    //!< Sort by number of received packets.
		TX_PACKETS,    //!< Sort by number of sent packets.
		RX_BYTES,      //!< Sort by number of received bytes.
		TX_BYTES,      //!< Sort by number of sent bytes.
		RX_SPEED,      //!< Sort by receive speed.
		TX_SPEED       //!< Sort by send speed.
	};

	/**
	 * @brief Network connection update flags.
	 */
	namespace EConnectionUpdateFlags
	{
		enum
		{
			PROTO       = (1 << 0),   //!< Connection protocol changed.
			PROTO_STATE = (1 << 1),   //!< Connection protocol state changed.
			SRC_ADDRESS = (1 << 2),   //!< Source address data changed.
			DST_ADDRESS = (1 << 3),   //!< Destination address data changed.
			SRC_PORT    = (1 << 4),   //!< Source port data changed.
			DST_PORT    = (1 << 5),   //!< Destination port data changed.
			RX_PACKETS  = (1 << 6),   //!< Number of received packets changed.
			TX_PACKETS  = (1 << 7),   //!< Number of sent packets changed.
			RX_BYTES    = (1 << 8),   //!< Number of received bytes changed.
			TX_BYTES    = (1 << 9),   //!< Number of sent bytes changed.
			RX_SPEED    = (1 << 10),  //!< Receive speed changed.
			TX_SPEED    = (1 << 11)   //!< Send speed changed.
		};
	};

	/**
	 * @brief UDP protocol.
	 */
	namespace UDP
	{
		/**
		 * @brief UDP connection state.
		 */
		enum EState
		{
			LISTEN  = -1,

			UNKNOWN = 0
		};

		KString StateToString( int state );
		EState StateToEnum( const KString & state );
	}

	/**
	 * @brief TCP protocol.
	 */
	namespace TCP
	{
		/**
		 * @brief TCP connection state.
		 */
		enum EState
		{
			LISTEN  = -1,

			UNKNOWN = 0,

			SYN_SENT,
			SYN_RECEIVED,
			ESTABLISHED,
			FIN_WAIT,
			FIN_WAIT_1,
			FIN_WAIT_2,
			CLOSE_WAIT,
			CLOSING,
			LAST_ACK,
			TIME_WAIT,
			CLOSED
		};

		KString StateToString( int state );
		EState StateToEnum( const KString & state );
	}

	/**
	 * @brief Network connection.
	 */
	class Connection
	{
		class Internal
		{
			struct Ports
			{
				const PortData *pSrcPort;
				const PortData *pDstPort;
			};

			union
			{
				Ports m_ports;
			};

			EConnectionType m_type;

			void copy( const Internal & other )
			{
				m_type = other.m_type;
				switch ( m_type )
				{
					case EConnectionType::UDP4:
					case EConnectionType::UDP6:
					case EConnectionType::TCP4:
					case EConnectionType::TCP6:
					{
						m_ports.pSrcPort = other.m_ports.pSrcPort;
						m_ports.pDstPort = other.m_ports.pDstPort;
						break;
					}
				}
			}

		public:
			Internal( EConnectionType type, const PortData & srcPort, const PortData & dstPort )
			: m_ports(),
			  m_type(type)
			{
				m_ports.pSrcPort = &srcPort;
				m_ports.pDstPort = &dstPort;
			}

			Internal( const Internal & other )
			{
				copy( other );
			}

			Internal & operator=( const Internal & other )
			{
				if ( this != &other )
				{
					copy( other );
				}
				return *this;
			}

			// no move
			Internal( Internal && ) = delete;
			Internal & operator=( Internal && ) = delete;

			~Internal()
			{
			}

			EConnectionType getType() const
			{
				return m_type;
			}

			EPortType getPortType() const
			{
				return m_ports.pSrcPort->getPortType();
			}

			const PortData & getSrcPort() const
			{
				return *m_ports.pSrcPort;
			}

			const PortData & getDstPort() const
			{
				return *m_ports.pDstPort;
			}

			bool hasPorts() const
			{
				switch ( m_type )
				{
					case EConnectionType::UDP4:
					case EConnectionType::UDP6:
					case EConnectionType::TCP4:
					case EConnectionType::TCP6:
					{
						return true;
					}
				}
				return false;
			}

			bool isEqual( const Internal & other ) const
			{
				if ( m_type == other.m_type )
				{
					switch ( m_type )
					{
						case EConnectionType::UDP4:
						case EConnectionType::UDP6:
						case EConnectionType::TCP4:
						case EConnectionType::TCP6:
						{
							if ( m_ports.pSrcPort == other.m_ports.pSrcPort
							  && m_ports.pDstPort == other.m_ports.pDstPort )
							{
								return true;
							}
							break;
						}
					}
				}
				return false;
			}

			size_t computeHash() const
			{
				size_t h = std::hash<int>()( static_cast<int>( m_type ) );
				switch ( m_type )
				{
					case EConnectionType::UDP4:
					case EConnectionType::UDP6:
					case EConnectionType::TCP4:
					case EConnectionType::TCP6:
					{
						HashCombine( h, std::hash<const void*>()( m_ports.pSrcPort ) );
						HashCombine( h, std::hash<const void*>()( m_ports.pDstPort ) );
						break;
					}
				}
				return h;
			}
		};

		const AddressData *m_pSrcAddr;
		const AddressData *m_pDstAddr;
		Internal m_internal;

	public:
		/**
		 * @brief Constructor.
		 * @param srcAddress Source address.
		 * @param srcPort Source port.
		 * @param dstAddress Destination address.
		 * @param dstPort Destination address.
		 */
		Connection( const AddressData & srcAddress, const PortData & srcPort,
		            const AddressData & dstAddress, const PortData & dstPort )
		: m_pSrcAddr(&srcAddress),
		  m_pDstAddr(&dstAddress),
		  m_internal(CreateType( srcAddress.getAddressType(), srcPort.getPortType() ), srcPort, dstPort)
		{
		}

		/**
		 * @brief Returns connection type.
		 * @return Connection type.
		 */
		EConnectionType getType() const
		{
			return m_internal.getType();
		}

		/**
		 * @brief Returns name of connection type.
		 * @return Connection type name.
		 */
		KString getTypeName() const
		{
			return TypeToString( m_internal.getType() );
		}

		/**
		 * @brief Checks if the connection has ports.
		 * @return True, if the connection has ports, otherwise false.
		 */
		bool hasPorts() const
		{
			return m_internal.hasPorts();
		}

		/**
		 * @brief Returns source address.
		 * @return Source address.
		 */
		const AddressData & getSrcAddr() const
		{
			return *m_pSrcAddr;
		}

		/**
		 * @brief Returns destination address.
		 * @return Destination address.
		 */
		const AddressData & getDstAddr() const
		{
			return *m_pDstAddr;
		}

		/**
		 * @brief Returns source port.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Source port.
		 */
		const PortData & getSrcPort() const
		{
			return m_internal.getSrcPort();
		}

		/**
		 * @brief Returns destination port.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Destination port.
		 */
		const PortData & getDstPort() const
		{
			return m_internal.getDstPort();
		}

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		EAddressType getAddrType() const
		{
			return m_pSrcAddr->getAddressType();
		}

		/**
		 * @brief Returns port type.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Port type.
		 */
		EPortType getPortType() const
		{
			return m_internal.getPortType();
		}

		/**
		 * @brief Compares the connection with another.
		 * @return True, if connections are equal, otherwise false.
		 */
		bool isEqual( const Connection & other ) const
		{
			return m_pSrcAddr == other.m_pSrcAddr
			    && m_pDstAddr == other.m_pDstAddr
			    && m_internal.isEqual( other.m_internal );
		}

		/**
		 * @brief Computes hash of the connection.
		 * @return Connection hash.
		 */
		size_t computeHash() const
		{
			size_t h = m_internal.computeHash();
			HashCombine( h, std::hash<const void*>()( m_pSrcAddr ) );
			HashCombine( h, std::hash<const void*>()( m_pDstAddr ) );
			return h;
		}

		/**
		 * @brief Obtains name of connection state.
		 * @param state Connection state.
		 * @return Connection state name.
		 */
		KString getStateName( int state ) const
		{
			return StateToString( state, m_internal.getType() );
		}

		/**
		 * @brief Converts address type and port type to connection type.
		 * @param addressType Address type.
		 * @param portType Port type.
		 * @return Connection type.
		 */
		static EConnectionType CreateType( EAddressType addressType, EPortType portType )
		{
			switch ( addressType )
			{
				case EAddressType::IP4:
				{
					switch ( portType )
					{
						case EPortType::UDP:
						{
							return EConnectionType::UDP4;
						}
						case EPortType::TCP:
						{
							return EConnectionType::TCP4;
						}
					}
					break;
				}
				case EAddressType::IP6:
				{
					switch ( portType )
					{
						case EPortType::UDP:
						{
							return EConnectionType::UDP6;
						}
						case EPortType::TCP:
						{
							return EConnectionType::TCP6;
						}
					}
					break;
				}
			}

			return static_cast<EConnectionType>( -1 );
		}

		/**
		 * @brief Compares connection types.
		 * @return True, if connection types are equal, otherwise false.
		 */
		static bool IsProtoEqual( EConnectionType a, EConnectionType b )
		{
			switch ( a )
			{
				case EConnectionType::UDP4:
				case EConnectionType::UDP6:
				{
					if ( b == EConnectionType::UDP4
					  || b == EConnectionType::UDP6 )
					{
						return true;
					}
					break;
				}
				case EConnectionType::TCP4:
				case EConnectionType::TCP6:
				{
					if ( b == EConnectionType::TCP4
					  || b == EConnectionType::TCP6 )
					{
						return true;
					}
					break;
				}
			}
			return false;
		}

		static KString TypeToString( EConnectionType type );
		static EConnectionType TypeToEnum( const KString & type );

		static KString ActionToString( EConnectionAction action );
		static EConnectionAction ActionToEnum( const KString & action );

		static KString StateToString( int state, EConnectionType type );
	};

	/**
	 * @brief Network connection traffic.
	 */
	struct ConnectionTraffic
	{
		uint64_t rxPackets;
		uint64_t txPackets;
		uint64_t rxBytes;
		uint64_t txBytes;
		uint64_t rxSpeed;
		uint64_t txSpeed;

		/**
		 * @brief Default constructor.
		 */
		ConnectionTraffic()
		: rxPackets(0),
		  txPackets(0),
		  rxBytes(0),
		  txBytes(0),
		  rxSpeed(0),
		  txSpeed(0)
		{
		}

		/**
		 * @brief Constructor.
		 * @param rxP Received packets count.
		 * @param txP Sent packets count.
		 * @param rxB Received bytes count.
		 * @param txB Sent bytes count.
		 * @param rxS Current receive speed.
		 * @param txS Current send speed.
		 */
		ConnectionTraffic( uint64_t rxP, uint64_t txP, uint64_t rxB, uint64_t txB, uint64_t rxS, uint64_t txS )
		: rxPackets(rxP),
		  txPackets(txP),
		  rxBytes(rxB),
		  txBytes(txB),
		  rxSpeed(rxS),
		  txSpeed(txS)
		{
		}
	};

	struct IConnectionUpdateCallback;

	/**
	 * @brief Network connection data.
	 */
	class ConnectionData
	{
		//! Data belongs to this connection.
		const Connection *m_pConnection;

		//! Traffic information.
		ConnectionTraffic m_traffic;
		//! Current connection state.
		int m_state;

		//! True, if the connection has ports, otherwise false.
		bool m_hasPorts;
		//! Connection type name.
		KString m_typeName;

	public:
		/**
		 * @brief Constructor.
		 * The connection must be stored somewhere else because this class holds only a pointer to it.
		 * @param connection The connection.
		 */
		ConnectionData( const Connection & connection )
		: m_pConnection(&connection),
		  m_traffic(),
		  m_state(0),
		  m_hasPorts(connection.hasPorts()),
		  m_typeName(connection.getTypeName())
		{
		}

		/**
		 * @brief Constructor.
		 * The connection must be stored somewhere else because this class holds only a pointer to it.
		 * @param connection The connection.
		 * @param traffic Connection traffic.
		 * @param state Current connection state.
		 */
		ConnectionData( const Connection & connection, const ConnectionTraffic & traffic, int state = 0 )
		: m_pConnection(&connection),
		  m_traffic(traffic),
		  m_state(state),
		  m_hasPorts(connection.hasPorts()),
		  m_typeName(connection.getTypeName())
		{
		}

		/**
		 * @brief Returns connection type.
		 * @return Connection type.
		 */
		EConnectionType getType() const
		{
			return m_pConnection->getType();
		}

		/**
		 * @brief Returns name of connection type.
		 * @return Connection type name.
		 */
		KString getTypeName() const
		{
			return m_typeName;
		}

		/**
		 * @brief Returns connection to which the data belongs.
		 * @return The connection.
		 */
		const Connection & getConnection() const
		{
			return *m_pConnection;
		}

		/**
		 * @brief Checks if the connection has ports.
		 * @return True, if the connection has ports, otherwise false.
		 */
		bool hasPorts() const
		{
			return m_hasPorts;
		}

		/**
		 * @brief Returns source address.
		 * @return Source address.
		 */
		const AddressData & getSrcAddr() const
		{
			return m_pConnection->getSrcAddr();
		}

		/**
		 * @brief Returns destination address.
		 * @return Destination address.
		 */
		const AddressData & getDstAddr() const
		{
			return m_pConnection->getDstAddr();
		}

		/**
		 * @brief Returns source port.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Source port.
		 */
		const PortData & getSrcPort() const
		{
			return m_pConnection->getSrcPort();
		}

		/**
		 * @brief Returns destination port.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Destination port.
		 */
		const PortData & getDstPort() const
		{
			return m_pConnection->getDstPort();
		}

		/**
		 * @brief Returns address type.
		 * @return Address type.
		 */
		EAddressType getAddrType() const
		{
			return m_pConnection->getAddrType();
		}

		/**
		 * @brief Returns port type.
		 * If the connection doesn't have ports, calling this function is undefined behavior.
		 * @return Port type.
		 */
		EPortType getPortType() const
		{
			return m_pConnection->getPortType();
		}

		/**
		 * @brief Returns current connection state.
		 * @return Connection state.
		 */
		int getState() const
		{
			return m_state;
		}

		/**
		 * @brief Returns name of current connection state.
		 * @return Connection state name.
		 */
		KString getStateName() const
		{
			return m_pConnection->getStateName( m_state );
		}

		/**
		 * @brief Returns number of received packets.
		 * @return Received packets count.
		 */
		uint64_t getRXPackets() const
		{
			return m_traffic.rxPackets;
		}

		/**
		 * @brief Returns number of received bytes.
		 * @return Received bytes count.
		 */
		uint64_t getRXBytes() const
		{
			return m_traffic.rxBytes;
		}

		/**
		 * @brief Returns number of sent packets.
		 * @return Sent packets count.
		 */
		uint64_t getTXPackets() const
		{
			return m_traffic.txPackets;
		}

		/**
		 * @brief Returns number of sent bytes.
		 * @return Sent bytes count.
		 */
		uint64_t getTXBytes() const
		{
			return m_traffic.txBytes;
		}

		/**
		 * @brief Returns current receive speed in bytes.
		 * @return Current receive speed.
		 */
		uint64_t getRXSpeed() const
		{
			return m_traffic.rxSpeed;
		}

		/**
		 * @brief Returns current send speed in bytes.
		 * @return Current send speed.
		 */
		uint64_t getTXSpeed() const
		{
			return m_traffic.txSpeed;
		}

		/**
		 * @brief Returns connection traffic data.
		 * @return Traffic data.
		 */
		const ConnectionTraffic & getTraffic() const
		{
			return m_traffic;
		}

		/**
		 * @brief Returns connection traffic data.
		 * @return Traffic data.
		 */
		ConnectionTraffic & getTraffic()
		{
			return m_traffic;
		}

		/**
		 * @brief Sets current connection state.
		 * @param state Connection state.
		 */
		void setState( int state )
		{
			m_state = state;
		}

		/**
		 * @brief Sets connection to which the data belongs.
		 * This function should be used only in ConnectionStorage class.
		 * @param connection The connection.
		 */
		void setConnection( const Connection & connection )
		{
			m_pConnection = &connection;
		}

		void serialize( rapidjson::Document & document, EConnectionAction action, int updateFlags = -1 ) const;

		static bool Deserialize( const rapidjson::Value & document, IConnectionUpdateCallback *callback );
	};

	/**
	 * @brief Connection update callback interface.
	 * This class is used by collector to add, remove, and update connections in the connection list. It is also used during
	 * connection deserialization.
	 */
	struct IConnectionUpdateCallback
	{
		virtual AddressData *getAddress( const IAddress & address, bool add = false ) = 0;

		virtual PortData *getPort( const Port & port, bool add = false ) = 0;

		virtual ConnectionData *find( const Connection & connection ) = 0;

		virtual ConnectionData *add( const Connection & connection ) = 0;

		virtual ConnectionData *add( const Connection & connection, const ConnectionTraffic & traffic, int state ) = 0;

		virtual void update( const ConnectionData & data, int updateFlags ) = 0;

		virtual void remove( const Connection & connection ) = 0;

		virtual void clear() = 0;
	};

	inline bool operator==( const Connection & a, const Connection & b )
	{
		return a.isEqual( b );
	}

	inline bool operator!=( const Connection & a, const Connection & b )
	{
		return ! (a == b);
	}
}

namespace std
{
	template<>
	struct hash<ctp::Connection>
	{
		using argument_type = ctp::Connection;
		using result_type = size_t;

		result_type operator()( const argument_type & v ) const
		{
			return v.computeHash();
		}
	};
}
