/**
 * @file
 * @brief Implementation of Client class.
 */

#include <stdexcept>

#include "Client.hpp"
#include "DataFlags.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "CmdLine.hpp"
#include "ConnectionList.hpp"
#include "Resolver.hpp"
#include "IUI.hpp"
#include "Exception.hpp"
#include "Util.hpp"

namespace ctp
{
	class ClientConnector
	{
	public:
		static constexpr uint16_t DEFAULT_PORT = 55555;

	private:
		ClientSession *m_pSession;
		std::string m_host;
		AddressPack m_resolvedAddresses;
		PortPack m_resolvedPorts;
		unsigned int m_addressIndex;
		unsigned int m_portIndex;
		const IAddress *m_currentAddress;
		uint16_t m_currentPort;

		static void HostResolveCallback(std::string & hostname, AddressPack & pack, void *param)
		{
			ClientConnector *self = static_cast<ClientConnector*>(param);

			if (pack.isEmpty())
			{
				std::string errMsg = "Unable to resolve server address '";
				errMsg += hostname;
				errMsg += "'";
				throw Exception(std::move(errMsg), "Client");
			}
			else
			{
				self->m_resolvedAddresses = std::move(pack);
				self->m_addressIndex = 0;
				self->tryNext();
			}
		}

		static void PortResolveCallback(std::string & service, EPortType, PortPack & pack, void *param)
		{
			ClientConnector *self = static_cast<ClientConnector*>(param);

			if (pack.isEmpty())
			{
				std::string errMsg = "Unable to resolve server port '";
				errMsg += service;
				errMsg += "'";
				throw Exception(std::move(errMsg), "Client");
			}
			else
			{
				self->m_resolvedPorts = std::move(pack);
				self->m_portIndex = 0;
				// resolve server host
				gApp->getResolver()->resolveHostname(self->m_host, HostResolveCallback, self);
			}
		}

	public:
		ClientConnector(ClientSession & session)
		: m_pSession(&session),
		  m_host(),
		  m_resolvedAddresses(),
		  m_resolvedPorts(),
		  m_addressIndex(),
		  m_portIndex(),
		  m_currentAddress(),
		  m_currentPort()
		{
			CmdLineArg *connectArg = gCmdLine->getArg("connect");
			if (!connectArg)
			{
				throw Exception("No server to connect", "Client");
			}

			m_host = connectArg->getValue();

			CmdLineArg *portArg = gCmdLine->getArg("port");
			if (portArg)
			{
				KString port = portArg->getValue();
				// resolve server port
				gApp->getResolver()->resolveService(port, EPortType::TCP, PortResolveCallback, this);
			}
			else
			{
				// resolve server host
				gApp->getResolver()->resolveHostname(m_host, HostResolveCallback, this);
			}
		}

		const std::string & getHost() const
		{
			return m_host;
		}

		const IAddress *getCurrentAddress() const
		{
			return m_currentAddress;
		}

		uint16_t getCurrentPort() const
		{
			return m_currentPort;
		}

		void tryNext()
		{
			if (m_resolvedAddresses.isEmpty())
			{
				return;
			}

			StreamSocket socket;
			while (!socket.isConnected())
			{
				m_currentAddress = nullptr;
				m_currentPort = DEFAULT_PORT;

				if (m_resolvedPorts.isEmpty())
				{
					if (m_addressIndex < m_resolvedAddresses.getSize())
					{
						m_currentAddress = &m_resolvedAddresses[m_addressIndex];
						m_addressIndex++;
					}
				}
				else
				{
					if (m_addressIndex >= m_resolvedAddresses.getSize())
					{
						m_portIndex++;
						m_addressIndex = 0;
					}

					m_currentAddress = &m_resolvedAddresses[m_addressIndex];
					m_addressIndex++;

					if (m_portIndex < m_resolvedPorts.getSize())
					{
						m_currentPort = m_resolvedPorts[m_portIndex].getNumber();
					}
					else
					{
						m_currentAddress = nullptr;
					}
				}

				if (!m_currentAddress)  // no more address-port combinations to try
				{
					std::string errMsg = "Unable to connect to '";
					errMsg += m_host;
					errMsg += "'";
					throw Exception(std::move(errMsg), "Client");
				}

				try
				{
					socket.connect(*m_currentAddress, m_currentPort, EStreamSocketType::TCP);
				}
				catch (const SocketException & e)
				{
					const std::string dest = Util::AddressPortToString(*m_currentAddress, m_currentPort);

					gLog->error("[Client] Connection to %s failed: %s", dest.c_str(), e.what());
				}
			}

			m_pSession->connect(std::move(socket));
		}
	};

	Client::Client()
	: m_context(this),
	  m_session(m_context),
	  m_pConnector(),
	  m_requestedDataFlags(),
	  m_requestedDataUpdateFlags(),
	  m_remainingDataFlags(),
	  m_isDataRequested(),
	  m_isSynchronized(),
	  m_isPaused()
	{
	}

	Client::~Client()
	{
	}

	void Client::init()
	{
		m_pConnector = std::make_unique<ClientConnector>(m_session);

		gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::CONNECT_STARTED);
	}

	void Client::onUpdate()
	{
		m_session.onUpdate();
	}

	void Client::disconnect()
	{
		gLog->info("[Client] Disconnecting...");

		if (m_pConnector)
		{
			// destroy connector
			m_pConnector.reset();
		}

		m_session.disconnect();

		gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::DISCONNECT_STARTED);
	}

	void Client::setPaused(bool paused)
	{
		if (m_isPaused != paused)
		{
			m_isPaused = paused;

			if (m_isPaused)
			{
				gLog->debug("[Client] Paused");
			}
			else
			{
				gLog->debug("[Client] Resumed");

				if (m_remainingDataFlags == 0 && !m_isDataRequested)
				{
					requestData();
				}
			}
		}
	}

	void Client::onSessionConnectionEstablished(ClientSession*)
	{
		if (m_pConnector)
		{
			m_session.setServerHostString(m_pConnector->getHost());
			// destroy connector
			m_pConnector.reset();
		}

		if (gLog->isMsgEnabled(Log::INFO))
		{
			gLog->info("[Client] Connection to %s (%s) established",
			  m_session.getServerEndpoint().toString().c_str(),
			  m_session.getServerHostString().c_str()
			);
		}

		gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::CONNECTION_ESTABLISHED);
	}

	void Client::onSessionEstablished(ClientSession*)
	{
		gLog->always("[Client] Connected to server %s at %s (%s)",
		  m_session.getServerName().c_str(),
		  m_session.getServerEndpoint().toString().c_str(),
		  m_session.getServerHostString().c_str()
		);

		gLog->info("[Client] Server version is %s (platform: %s)",
		  m_session.getServerVersionString().c_str(),
		  m_session.getServerPlatformName().c_str()
		);

		m_requestedDataFlags = EDataFlags::CONNECTION;
		m_requestedDataUpdateFlags = 0;
		m_remainingDataFlags = 0;
		requestData();

		m_isSynchronized = false;

		gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::SESSION_ESTABLISHED);
	}

	void Client::onSessionDisconnect(ClientSession*)
	{
		std::string text;
		if (m_session.hasDisconnectError())
		{
			if (m_session.getDisconnectReason() == EDisconnectReason::UNKNOWN)
			{
				text = "Unknown reason";  // better than "?"
			}
			else
			{
				text = m_session.getDisconnectReasonName();
			}
			text += ": ";
			text += m_session.getDisconnectErrorString();
		}
		else
		{
			text = m_session.getDisconnectReasonName();
		}

		if (m_pConnector)
		{
			const IAddress & address = *m_pConnector->getCurrentAddress();
			const std::string dest = Util::AddressPortToString(address, m_pConnector->getCurrentPort());

			gLog->error("[Client] Connection to %s failed: %s",
			  dest.c_str(),
			  text.c_str()
			);

			m_pConnector->tryNext();
		}
		else
		{
			gLog->always("[Client] Disconnected from %s (%s): %s",
			  m_session.getServerEndpoint().toString().c_str(),
			  m_session.getServerHostString().c_str(),
			  text.c_str()
			);

			m_isSynchronized = false;
			m_isDataRequested = false;

			gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::DISCONNECTED);
		}

		if (m_session.getDisconnectReason() == EDisconnectReason::USER_DECISION)
		{
			gApp->quit();
		}
	}

	void Client::onSessionServerTick(ClientSession*)
	{
		gLog->debug("[Client] Server update tick (%u)", m_session.getCurrentTimestamp());

		gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::SERVER_UPDATE_TICK);

		if (m_remainingDataFlags)
		{
			m_remainingDataFlags = 0;
			m_requestedDataUpdateFlags = 0;

			setSynchronized(false);

			if (!m_isPaused)
			{
				requestData();
			}
		}
		else if (m_session.isDataRequestInProgress() || m_isPaused)
		{
			setSynchronized(false);
		}
		else
		{
			m_remainingDataFlags = m_requestedDataFlags;
		}

		m_isDataRequested = false;
	}

	void Client::onSessionDataStatus(ClientSession*, bool isDifferent)
	{
		const int dataFlags = m_session.getDataFlags();
		const int dataUpdateFlags = m_session.getDataUpdateFlags();

		gLog->debug("[Client] %s data status: %d %d", (isDifferent) ? "New" : "Same", dataFlags, dataUpdateFlags);

		m_requestedDataFlags &= dataFlags;
		m_requestedDataUpdateFlags &= dataUpdateFlags;

		if (m_requestedDataUpdateFlags && !m_isSynchronized)
		{
			m_requestedDataUpdateFlags = 0;
			requestData();
		}

		if (isDifferent)
		{
			gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::NEW_DATA_AVAILABLE);
		}
	}

	void Client::onSessionData(ClientSession*, int type, bool isUpdate, rapidjson::Value & data)
	{
		gLog->debug("[Client] Received data%s: %d", (isUpdate) ? " update" : "", type);

		if ((isUpdate && !m_isSynchronized) || !(type & m_remainingDataFlags) || m_isPaused)
		{
			return;
		}

		if (type & EDataFlags::CONNECTION)
		{
			gLog->debug("[Client] CONNECTION data (%zu)", data.Size());

			if (!data.IsArray())
			{
				throw std::invalid_argument("Data is not an array");
			}

			if (!isUpdate)
			{
				gApp->getConnectionList()->clear();
			}

			// deserialize
			for (auto it = data.Begin(); it != data.End(); ++it)
			{
				ConnectionData::Deserialize(*it, gApp->getConnectionList());
			}

			if (gApp->hasUI())
			{
				gApp->getUI()->refreshConnectionList();
			}

			m_remainingDataFlags &= ~EDataFlags::CONNECTION;
		}
		else
		{
			gLog->warning("[Client] Unknown data of type %d, ignoring...", type);
			return;
		}

		gLog->debug("[Client] Data deserialization done");

		if (!m_remainingDataFlags)
		{
			if (!m_isSynchronized)
			{
				setSynchronized(true);
				m_requestedDataUpdateFlags = m_requestedDataFlags;
			}

			requestData();
		}
	}

	void Client::requestData()
	{
		gLog->debug("[Client] Requesting data: %d %d", m_requestedDataFlags, m_requestedDataUpdateFlags);
		m_session.requestData(m_requestedDataFlags, m_requestedDataUpdateFlags);
		m_isDataRequested = true;
	}

	void Client::setSynchronized(bool isSynchronized)
	{
		if (m_isSynchronized != isSynchronized)
		{
			m_isSynchronized = isSynchronized;
			gLog->debug("[Client] Synchronization state changed to %s", (m_isSynchronized) ? "true" : "false");
			gApp->getEventSystem()->dispatch<ClientEvent>(ClientEvent::SYNC_STATE_CHANGED);
		}
	}
}
