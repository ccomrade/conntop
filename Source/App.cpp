/**
 * @file
 * @brief Implementation of App class.
 */

#include "App.hpp"
#include "Log.hpp"
#include "CmdLine.hpp"
#include "Events.hpp"
#include "Server.hpp"
#include "ICollector.hpp"
#include "Exception.hpp"

#ifndef CONNTOP_DEDICATED
#include "ConnectionList.hpp"
#include "Client.hpp"
#include "GeoIP.hpp"
#include "Resolver.hpp"
#include "IUI.hpp"
#endif

#ifdef CONNTOP_COLLECTOR_NETFILTER
#include "Collector_Netfilter/CCollector.hpp"
#endif

#ifdef CONNTOP_UI_CURSES
#include "UI_Curses/CUI.hpp"
#endif

#ifdef CONNTOP_COLLECTOR_NETFILTER
using CCollector = CCollector_Netfilter;
#endif

#ifdef CONNTOP_UI_CURSES
using CUI = CUI_Curses;
#endif

struct AppEvent
{
	static constexpr int ID = EGlobalEventID::APP_INTERNAL_EVENT;

	enum EType
	{
		QUIT,
		FATAL_ERROR
	};

private:
	EType m_type;
	std::string m_fatalErrorMessage;
	const char *m_fatalErrorOrigin;

public:
	AppEvent(EType type)
	: m_type(type),
	  m_fatalErrorMessage(),
	  m_fatalErrorOrigin()
	{
	}

	EType getType() const
	{
		return m_type;
	}

	const std::string & getFatalErrorMessageString() const
	{
		return m_fatalErrorMessage;
	}

	const char *getFatalErrorOrigin() const
	{
		return m_fatalErrorOrigin;
	}

	void setFatalError(std::string && message, const char *origin = nullptr)
	{
		m_fatalErrorMessage = std::move(message);
		m_fatalErrorOrigin = origin;
	}
};

struct AppEventCallback : public IEventCallback<AppEvent>
{
private:
	std::string m_fatalErrorMessage;
	const char *m_fatalErrorOrigin;
	bool m_hasFatalError;

public:
	AppEventCallback()
	: m_fatalErrorMessage(),
	  m_fatalErrorOrigin(),
	  m_hasFatalError(false)
	{
		gApp->getEventSystem()->registerCallback<AppEvent>(this);
	}

	~AppEventCallback()
	{
		gApp->getEventSystem()->removeCallback<AppEvent>(this);
	}

	void onEvent(const AppEvent & event) override
	{
		switch (event.getType())
		{
			case AppEvent::QUIT:
			{
				gApp->getEventSystem()->stop();
				break;
			}
			case AppEvent::FATAL_ERROR:
			{
				m_fatalErrorMessage = event.getFatalErrorMessageString();
				m_fatalErrorOrigin = event.getFatalErrorOrigin();
				m_hasFatalError = true;
				gApp->getEventSystem()->stop();
				break;
			}
		}
	}

	bool hasFatalError() const
	{
		return m_hasFatalError;
	}

	Exception buildException()
	{
		return Exception(std::move(m_fatalErrorMessage), m_fatalErrorOrigin, false);
	}
};

struct UpdateEventCallback : public IEventCallback<UpdateEvent>
{
	UpdateEventCallback()
	{
		gApp->getEventSystem()->registerCallback<UpdateEvent>(this);
	}

	~UpdateEventCallback()
	{
		gApp->getEventSystem()->removeCallback<UpdateEvent>(this);
	}

	void onEvent(const UpdateEvent &) override
	{
		if (gApp->isServer())
		{
			gApp->getServer()->onUpdate();
		}
	#ifndef CONNTOP_DEDICATED
		else if (gApp->isClient())
		{
			gApp->getClient()->onUpdate();
		}
		else if (gApp->hasCollector())
		{
			gApp->getCollector()->onUpdate();

			if (gApp->hasUI())
			{
				gApp->getUI()->refreshConnectionList();
			}
		}
	#endif
	}
};

App::App()
: m_pEventSystem(),
  m_pPollSystem(),
#ifndef CONNTOP_DEDICATED
  m_pConnectionList(),
  m_pClient(),
  m_pGeoIP(),
  m_pResolver(),
  m_pUI(),
#endif
  m_pServer(),
  m_pCollector()
{
	gApp = this;

	m_pEventSystem = std::make_unique<EventSystem>();
	m_pPollSystem = std::make_unique<PollSystem>();
}

App::~App()
{
}

void App::launch()
{
#ifndef CONNTOP_DEDICATED
	if (gCmdLine->hasArg("server"))
	{
		m_pCollector = std::make_unique<CCollector>();
		m_pServer = std::make_unique<Server>();
	}
	else
	{
		if (!gCmdLine->hasArg("no-geoip"))
		{
			m_pGeoIP = std::make_unique<GeoIP>();
		}

		m_pResolver = std::make_unique<Resolver>();
		m_pConnectionList = std::make_unique<ConnectionList>();

		if (gCmdLine->hasArg("connect"))
		{
			m_pClient = std::make_unique<Client>();
		}
		else
		{
			m_pCollector = std::make_unique<CCollector>();
		}

		m_pUI = std::make_unique<CUI>();
	}
#else
	m_pCollector = std::make_unique<CCollector>();
	m_pServer = std::make_unique<Server>();
#endif

	AppEventCallback appEventCallback;
	UpdateEventCallback updateEventCallback;

	if (hasCollector())
	{
		if (isServer())
		{
			m_pCollector->init(m_pServer->getConnectionUpdateCallback());
		}
	#ifndef CONNTOP_DEDICATED
		else
		{
			m_pCollector->init(getConnectionList());
		}
	#endif
	}

	if (isServer())
	{
		m_pServer->init();
	}

#ifndef CONNTOP_DEDICATED
	if (isClient())
	{
		m_pClient->init();
	}

	if (hasUI())
	{
		m_pUI->init();
	}
#endif

	gLog->info("[App] Initialization completed");

#ifndef CONNTOP_DEDICATED
	if (hasCollector() && hasUI())
	{
		m_pCollector->onUpdate();
		m_pUI->refreshConnectionList();
		gLog->debug("[App] First update done");
	}
#endif

	// application update loop
	m_pEventSystem->run();

	if (appEventCallback.hasFatalError())
	{
		throw appEventCallback.buildException();
	}
}

void App::quit()
{
	m_pEventSystem->dispatch<AppEvent>(AppEvent::QUIT);
}

void App::fatalError(std::string errorMessage, const char *origin, bool log)
{
	if (log)
	{
		if (origin)
		{
			gLog->error("[%s] %s", origin, errorMessage.c_str());
		}
		else
		{
			gLog->error("%s", errorMessage.c_str());
		}
	}

	AppEvent event(AppEvent::FATAL_ERROR);
	event.setFatalError(std::move(errorMessage), origin);

	m_pEventSystem->dispatch<AppEvent>(std::move(event));
}
