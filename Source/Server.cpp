/**
 * @file
 * @brief Implementation of Server class.
 */

#include "Server.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "CmdLine.hpp"
#include "ICollector.hpp"
#include "Exception.hpp"

Server::Server()
: m_context(this),
  m_clients(),
  m_sockets(),
  m_availableDataFlags(0),
  m_connectionStorage(),
  m_serializedConnections(),
  m_serializedConnectionUpdates(),
  m_connectionUpdateSerializer(m_connectionStorage)
{
	if (gApp->hasCollector())
	{
		m_availableDataFlags |= EDataFlags::CONNECTION;
	}
}

Server::~Server()
{
	close();
}

void Server::init()
{
	open();
}

void Server::onUpdate()
{
	const ClientServerProtocol & protocol = m_context.getProtocol();

	bool requiresConnections = false;
	bool requiresConnectionUpdates = false;

	for (auto it = m_clients.begin(); it != m_clients.end();)
	{
		if (it->getState() == ESessionState::DISCONNECTED)
		{
			it = m_clients.erase(it);
		}
		else
		{
			if (it->isSendingData())
			{
				it->stopSendingData();
			}
			else
			{
				if (it->getDataFlags() & EDataFlags::CONNECTION)
				{
					if (it->getDataUpdateFlags() & EDataFlags::CONNECTION)
					{
						requiresConnectionUpdates = true;
					}
					else
					{
						requiresConnections = true;
					}
				}
			}

			++it;
		}
	}

	m_context.onUpdate();

	bool connectionsAvailable = false;
	bool connectionUpdatesAvailable = false;

	if (m_availableDataFlags & EDataFlags::CONNECTION)
	{
		gApp->getCollector()->onUpdate();

		if (m_connectionUpdateSerializer.isSerializationEnabled())
		{
			m_serializedConnectionUpdates = m_connectionUpdateSerializer.build(protocol);

			if (requiresConnectionUpdates)
			{
				connectionUpdatesAvailable = true;
			}
		}
		else
		{
			m_serializedConnectionUpdates.clear();
		}

		if (requiresConnections)
		{
			ConnectionStorageSerializer connectionStorageSerializer(m_connectionStorage);
			m_serializedConnections = connectionStorageSerializer.build(protocol);
			connectionsAvailable = true;
		}
		else
		{
			m_serializedConnections.clear();
		}
	}

	for (ServerSession & client : m_clients)
	{
		const int dataFlags = client.getDataFlags();
		const int dataUpdateFlags = client.getDataUpdateFlags();

		client.onUpdate();

		if (client.getState() != ESessionState::CONNECTED)
		{
			continue;
		}

		if (dataFlags & EDataFlags::CONNECTION)
		{
			if (connectionUpdatesAvailable && dataUpdateFlags & EDataFlags::CONNECTION)
			{
				client.sendData(m_serializedConnectionUpdates);
			}
			else if (connectionsAvailable)
			{
				client.sendData(m_serializedConnections);
			}
		}
	}
}

void Server::open()
{
	CmdLineArg *portArg = gCmdLine->getArg("port");
	KString port = (portArg) ? portArg->getValue() : "55555";

	if (gCmdLine->hasArg("listen-any"))
	{
		StreamServerSocket socketIP4;
		try
		{
			socketIP4.openAny(EAddressType::IP4, port, EStreamSocketType::TCP);
		}
		catch (const SocketException & e)
		{
			std::string errMsg = "Unable to listen on 0.0.0.0:";
			errMsg += port;
			errMsg += ": ";
			errMsg += e.getString();
			throw Exception(std::move(errMsg), "Server");
		}

		StreamServerSocket socketIP6;
		try
		{
			socketIP6.openAny(EAddressType::IP6, port, EStreamSocketType::TCP);
		}
		catch (const SocketException & e)
		{
			std::string errMsg = "Unable to listen on [::]:";
			errMsg += port;
			errMsg += ": ";
			errMsg += e.getString();
			throw Exception(std::move(errMsg), "Server");
		}

		addSocket(std::move(socketIP4));
		addSocket(std::move(socketIP6));
	}
	else if (gCmdLine->hasArg("listen"))
	{
		CmdLineArg *listenArg = gCmdLine->getArg("listen");
		for (const KString & address : listenArg->getAllValues())
		{
			StreamServerSocket socket;
			try
			{
				socket.open(address, port, EStreamSocketType::TCP);
			}
			catch (const SocketException & e)
			{
				std::string errMsg = "Unable to listen on ";
				errMsg += address;
				errMsg += ":";
				errMsg += port;
				errMsg += ": ";
				errMsg += e.getString();
				throw Exception(std::move(errMsg), "Server");
			}

			addSocket(std::move(socket));
		}
	}
	else
	{
		StreamServerSocket socketIP4;
		try
		{
			socketIP4.openLocalhost(EAddressType::IP4, port, EStreamSocketType::TCP);
		}
		catch (const SocketException & e)
		{
			std::string errMsg = "Unable to listen on 127.0.0.1:";
			errMsg += port;
			errMsg += ": ";
			errMsg += e.getString();
			throw Exception(std::move(errMsg), "Server");
		}

		StreamServerSocket socketIP6;
		try
		{
			socketIP6.openLocalhost(EAddressType::IP6, port, EStreamSocketType::TCP);
		}
		catch (const SocketException & e)
		{
			std::string errMsg = "Unable to listen on [::1]:";
			errMsg += port;
			errMsg += ": ";
			errMsg += e.getString();
			throw Exception(std::move(errMsg), "Server");
		}

		addSocket(std::move(socketIP4));
		addSocket(std::move(socketIP6));
	}
}

void Server::close()
{
	for (StreamServerSocket & serverSocket : m_sockets)
	{
		if (serverSocket.isOpen())
		{
			gApp->getPollSystem()->remove(serverSocket);
			serverSocket.close_nothrow();
		}
	}
	m_sockets.clear();
}

void Server::addSocket(StreamServerSocket && socket)
{
	m_sockets.emplace_back(std::move(socket));

	auto PollCallback = [this](int flags, void *param) -> void
	{
		pollHandler(flags, *static_cast<StreamServerSocket*>(param));
	};

	StreamServerSocket & newSocket = m_sockets.back();

	gApp->getPollSystem()->add(newSocket, EPollFlags::INPUT, PollCallback, &newSocket);
}

void Server::pollHandler(int flags, StreamServerSocket & serverSocket)
{
	if (flags & EPollFlags::INPUT)
	{
		StreamSocket clientSocket;
		try
		{
			clientSocket = serverSocket.accept();
		}
		catch (const SocketException & e)
		{
			gLog->error("[Server] Unable to accept client connection: %s", e.what());
		}

		if (clientSocket.isConnected())
		{
			m_clients.emplace_back(std::move(clientSocket), m_context);
		}
	}

	if (flags & EPollFlags::ERROR)
	{
		throw Exception("Server socket poll failed", "Server");
	}

	if (serverSocket.isOpen())
	{
		gApp->getPollSystem()->reset(serverSocket, EPollFlags::INPUT);
	}
}

void Server::onSessionEstablished(ServerSession *session)
{
	gLog->always("[Server] Client connected - %s (%s %s) | %s",
	  session->getClientName().c_str(),
	  session->getClientVersionString().c_str(),
	  session->getClientPlatformName().c_str(),
	  session->getClientEndpoint().toString().c_str()
	);
}

void Server::onSessionDisconnect(ServerSession *session)
{
	std::string text;
	if (session->hasDisconnectError())
	{
		if (session->getDisconnectReason() == EDisconnectReason::UNKNOWN)
		{
			text = "Unknown reason";  // better than "?"
		}
		else
		{
			text = session->getDisconnectReasonName();
		}
		text += ": ";
		text += session->getDisconnectErrorString();
	}
	else
	{
		text = session->getDisconnectReasonName();
	}

	gLog->always("[Server] Client disconnected - %s | %s | %s",
	  session->getClientName().c_str(),
	  session->getClientEndpoint().toString().c_str(),
	  text.c_str()
	);
}

void Server::onSessionDataRequest(ServerSession *session, int dataFlags, int dataUpdateFlags)
{
	dataFlags &= m_availableDataFlags;
	dataUpdateFlags &= m_availableDataFlags;

	session->sendDataStatus(dataFlags, dataUpdateFlags);

	if (dataFlags & EDataFlags::CONNECTION && dataUpdateFlags & EDataFlags::CONNECTION)
	{
		m_connectionUpdateSerializer.setSerializationEnabled(true);
	}
}
