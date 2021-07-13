/**
 * @file
 * @brief App class.
 */

#pragma once

#include <string>
#include <memory>

#include "GlobalEnvironment.hpp"
#include "EventSystem.hpp"
#include "PollSystem.hpp"
#include "conntop_config.h"

class ConnectionList;
class Client;
class Server;
class GeoIP;
class Resolver;

struct ICollector;
struct IUI;

class App
{
	std::unique_ptr<EventSystem> m_pEventSystem;
	std::unique_ptr<PollSystem> m_pPollSystem;
#ifndef CONNTOP_DEDICATED
	std::unique_ptr<ConnectionList> m_pConnectionList;
	std::unique_ptr<Client> m_pClient;
	std::unique_ptr<GeoIP> m_pGeoIP;
	std::unique_ptr<Resolver> m_pResolver;
	std::unique_ptr<IUI> m_pUI;
#endif
	std::unique_ptr<Server> m_pServer;
	std::unique_ptr<ICollector> m_pCollector;

public:
	App();
	~App();

	EventSystem *getEventSystem()
	{
		return m_pEventSystem.get();
	}

	PollSystem *getPollSystem()
	{
		return m_pPollSystem.get();
	}

	bool isClient() const
	{
	#ifndef CONNTOP_DEDICATED
		return m_pClient.get() != nullptr;
	#else
		return false;
	#endif
	}

	bool isServer() const
	{
		return m_pServer.get() != nullptr;
	}

	bool hasConnectionList() const
	{
	#ifndef CONNTOP_DEDICATED
		return m_pConnectionList.get() != nullptr;
	#else
		return false;
	#endif
	}

	bool hasGeoIP() const
	{
	#ifndef CONNTOP_DEDICATED
		return m_pGeoIP.get() != nullptr;
	#else
		return false;
	#endif
	}

	bool hasResolver() const
	{
	#ifndef CONNTOP_DEDICATED
		return m_pResolver.get() != nullptr;
	#else
		return false;
	#endif
	}

	bool hasUI() const
	{
	#ifndef CONNTOP_DEDICATED
		return m_pUI.get() != nullptr;
	#else
		return false;
	#endif
	}

	bool hasCollector() const
	{
		return m_pCollector.get() != nullptr;
	}

	Client *getClient()
	{
	#ifndef CONNTOP_DEDICATED
		return m_pClient.get();
	#else
		return nullptr;
	#endif
	}

	Server *getServer()
	{
		return m_pServer.get();
	}

	ConnectionList *getConnectionList()
	{
	#ifndef CONNTOP_DEDICATED
		return m_pConnectionList.get();
	#else
		return nullptr;
	#endif
	}

	GeoIP *getGeoIP()
	{
	#ifndef CONNTOP_DEDICATED
		return m_pGeoIP.get();
	#else
		return nullptr;
	#endif
	}

	Resolver *getResolver()
	{
	#ifndef CONNTOP_DEDICATED
		return m_pResolver.get();
	#else
		return nullptr;
	#endif
	}

	IUI *getUI()
	{
	#ifndef CONNTOP_DEDICATED
		return m_pUI.get();
	#else
		return nullptr;
	#endif
	}

	ICollector *getCollector()
	{
		return m_pCollector.get();
	}

	void launch();

	void quit();

	void fatalError(std::string errorMessage, const char *origin = nullptr, bool log = true);
};
