/**
 * @file
 * @brief Server class.
 */

#pragma once

#include <deque>

#include "DataFlags.hpp"
#include "ClientServerProtocol.hpp"
#include "ConnectionStorage.hpp"
#include "Sockets.hpp"

namespace ctp
{
	struct ConnectionStorageSerializer : public ServerDataSerializer<ConnectionData>
	{
		ConnectionStorageSerializer(const ConnectionStorage & storage)
		{
			for (auto it = storage.begin(); it != storage.end(); ++it)
			{
				addItem(it->second, EConnectionAction::CREATE);
			}
		}

		SerializedServerMessage build(const ClientServerProtocol & protocol)
		{
			return buildMessage(protocol, EDataFlags::CONNECTION, false);
		}
	};

	class ConnectionUpdateSerializer : public IConnectionUpdateCallback
	{
		struct UpdateSerializer : public ServerDataSerializer<ConnectionData>
		{
			UpdateSerializer()
			{
			}

			void add(const ConnectionData & connection, EConnectionAction action, int updateFlags = -1)
			{
				addItem(connection, action, updateFlags);
			}

			SerializedServerMessage build(const ClientServerProtocol & protocol)
			{
				return buildMessage(protocol, EDataFlags::CONNECTION, true);
			}
		};

		ConnectionStorage *m_pStorage;
		UpdateSerializer m_serializer;
		bool m_isSerializationEnabled;

	public:
		ConnectionUpdateSerializer(ConnectionStorage & storage)
		: m_pStorage(&storage),
		  m_serializer(),
		  m_isSerializationEnabled(true)
		{
		}

		bool isSerializationEnabled() const
		{
			return m_isSerializationEnabled;
		}

		void setSerializationEnabled(bool enable)
		{
			if (m_isSerializationEnabled && !enable)
			{
				m_serializer.clear();
			}

			m_isSerializationEnabled = enable;
		}

		SerializedServerMessage build(const ClientServerProtocol & protocol)
		{
			return m_serializer.build(protocol);
		}

		// IConnectionUpdateCallback

		AddressData *getAddress(const IAddress & address, bool add) override
		{
			switch (address.getType())
			{
				case EAddressType::IP4:
				{
					return (add) ?
					  m_pStorage->addIP4Address(static_cast<const AddressIP4&>(address)).first :
					  m_pStorage->getIP4Address(static_cast<const AddressIP4&>(address));
				}
				case EAddressType::IP6:
				{
					return (add) ?
					  m_pStorage->addIP6Address(static_cast<const AddressIP6&>(address)).first :
					  m_pStorage->getIP6Address(static_cast<const AddressIP6&>(address));
				}
			}
			return nullptr;
		}

		PortData *getPort(const Port & port, bool add) override
		{
			return (add) ? m_pStorage->addPort(port).first : m_pStorage->getPort(port);
		}

		ConnectionData *find(const Connection & connection) override
		{
			return m_pStorage->getConnection(connection);
		}

		ConnectionData *add(const Connection & connection) override
		{
			auto result = m_pStorage->addConnection(connection);
			ConnectionData *pData = result.first;
			if (result.second && m_isSerializationEnabled)
			{
				m_serializer.add(*pData, EConnectionAction::CREATE);
			}
			return pData;
		}

		ConnectionData *add(const Connection & connection, const ConnectionTraffic & traffic, int state) override
		{
			auto result = m_pStorage->addConnection(connection, traffic, state);
			ConnectionData *pData = result.first;
			if (result.second && m_isSerializationEnabled)
			{
				m_serializer.add(*pData, EConnectionAction::CREATE);
			}
			return pData;
		}

		void update(const ConnectionData & data, int updateFlags) override
		{
			if (m_isSerializationEnabled)
			{
				m_serializer.add(data, EConnectionAction::UPDATE, updateFlags);
			}
		}

		void remove(const Connection & connection) override
		{
			ConnectionData *pData = m_pStorage->getConnection(connection);
			if (pData)
			{
				if (m_isSerializationEnabled)
				{
					m_serializer.add(*pData, EConnectionAction::REMOVE);
				}
				m_pStorage->removeConnection(connection);
			}
		}

		void clear() override
		{
			m_pStorage->clearConnections();
			setSerializationEnabled(false);
		}
	};

	class Server final : public IServerSessionCallback
	{
		ServerContext m_context;
		std::deque<ServerSession> m_clients;
		std::deque<StreamServerSocket> m_sockets;
		int m_availableDataFlags;
		ConnectionStorage m_connectionStorage;
		SerializedServerMessage m_serializedConnections;
		SerializedServerMessage m_serializedConnectionUpdates;
		ConnectionUpdateSerializer m_connectionUpdateSerializer;

		void open();
		void close();
		void addSocket(StreamServerSocket && socket);
		void pollHandler(int flags, StreamServerSocket & serverSocket);

		// IServerSessionCallback

		void onSessionEstablished(ServerSession *session) override;
		void onSessionDisconnect(ServerSession *session) override;
		void onSessionDataRequest(ServerSession *session, int dataFlags, int dataUpdateFlags) override;

		friend class ServerSession;  // IServerSessionCallback functions are private

	public:
		Server();
		~Server();

		void init();

		void onUpdate();

		IConnectionUpdateCallback *getConnectionUpdateCallback()
		{
			return &m_connectionUpdateSerializer;
		}
	};
}
