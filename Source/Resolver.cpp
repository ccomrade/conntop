/**
 * @file
 * @brief Implementation of Resolver class.
 */

#include "Resolver.hpp"
#include "GeoIP.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "Thread.hpp"
#include "Events.hpp"
#include "CmdLine.hpp"

#include "readerwriterqueue/readerwriterqueue.h"

enum struct EResolverRequest
{
	HOSTNAME,
	SERVICE,
	ADDRESS,
	PORT
};

struct IResolverRequest
{
	virtual ~IResolverRequest() = default;

	virtual EResolverRequest getType() const = 0;
};

struct ResolverEvent
{
	static constexpr int ID = EGlobalEventID::RESOLVER_INTERNAL_EVENT;

private:
	std::unique_ptr<IResolverRequest> m_pRequest;

public:
	ResolverEvent(std::unique_ptr<IResolverRequest> && pRequest)
	: m_pRequest(std::move(pRequest))
	{
	}

	bool isEmpty() const
	{
		return !m_pRequest;
	}

	const IResolverRequest & getRequest() const
	{
		return *m_pRequest;
	}
};

class HostnameRequest : public IResolverRequest
{
	std::string m_hostname;
	Resolver::CallbackHostname m_callback;
	void *m_callbackParam;
	AddressPack m_addressPack;

public:
	HostnameRequest(std::string && hostname, const Resolver::CallbackHostname & callback, void *param)
	: m_hostname(std::move(hostname)),
	  m_callback(callback),
	  m_callbackParam(param),
	  m_addressPack()
	{
	}

	EResolverRequest getType() const override
	{
		return EResolverRequest::HOSTNAME;
	}

	Resolver::CallbackHostname & getCallback()
	{
		return m_callback;
	}

	void *getCallbackParam()
	{
		return m_callbackParam;
	}

	std::string & getHostname()
	{
		return m_hostname;
	}

	AddressPack & getAddressPack()
	{
		return m_addressPack;
	}
};

class ServiceRequest : public IResolverRequest
{
	std::string m_service;
	EPortType m_portType;
	Resolver::CallbackService m_callback;
	void *m_callbackParam;
	PortPack m_portPack;

public:
	ServiceRequest(std::string && service, EPortType type, const Resolver::CallbackService & callback, void *param)
	: m_service(std::move(service)),
	  m_portType(type),
	  m_callback(callback),
	  m_callbackParam(param),
	  m_portPack()
	{
	}

	EResolverRequest getType() const override
	{
		return EResolverRequest::SERVICE;
	}

	Resolver::CallbackService & getCallback()
	{
		return m_callback;
	}

	void *getCallbackParam()
	{
		return m_callbackParam;
	}

	std::string & getService()
	{
		return m_service;
	}

	EPortType getPortType()
	{
		return m_portType;
	}

	PortPack & getPortPack()
	{
		return m_portPack;
	}
};

class AddressRequest : public IResolverRequest
{
	AddressData *m_pAddressData;
	Resolver::CallbackAddress m_callback;
	void *m_callbackParam;
	ResolvedAddress m_resolvedData;

public:
	AddressRequest(AddressData & addressData, const Resolver::CallbackAddress & callback, void *param)
	: m_pAddressData(&addressData),
	  m_callback(callback),
	  m_callbackParam(param),
	  m_resolvedData()
	{
	}

	EResolverRequest getType() const override
	{
		return EResolverRequest::ADDRESS;
	}

	Resolver::CallbackAddress & getCallback()
	{
		return m_callback;
	}

	void *getCallbackParam()
	{
		return m_callbackParam;
	}

	AddressData & getAddressData()
	{
		return *m_pAddressData;
	}

	ResolvedAddress & getResolvedData()
	{
		return m_resolvedData;
	}
};

class PortRequest : public IResolverRequest
{
	PortData *m_pPortData;
	Resolver::CallbackPort m_callback;
	void *m_callbackParam;
	ResolvedPort m_resolvedData;

public:
	PortRequest(PortData & portData, const Resolver::CallbackPort & callback, void *param)
	: m_pPortData(&portData),
	  m_callback(callback),
	  m_callbackParam(param),
	  m_resolvedData()
	{
	}

	EResolverRequest getType() const override
	{
		return EResolverRequest::PORT;
	}

	Resolver::CallbackPort & getCallback()
	{
		return m_callback;
	}

	void *getCallbackParam()
	{
		return m_callbackParam;
	}

	PortData & getPortData()
	{
		return *m_pPortData;
	}

	ResolvedPort & getResolvedData()
	{
		return m_resolvedData;
	}
};

class Resolver::Impl final : public IEventCallback<ResolverEvent>
{
	moodycamel::BlockingReaderWriterQueue<std::unique_ptr<IResolverRequest>> m_requestQueue;
	Thread m_resolverThread;
	bool m_isRunning;
	bool m_isAddressHostnameEnabled;
	bool m_isPortServiceEnabled;

	void resolverLoop()  // executed by resolver thread
	{
		while (m_isRunning)
		{
			std::unique_ptr<IResolverRequest> pRequest;
			m_requestQueue.wait_dequeue(pRequest);

			if (!pRequest)
			{
				continue;
			}

			switch (pRequest->getType())
			{
				case EResolverRequest::HOSTNAME:
				{
					processHostname(static_cast<HostnameRequest&>(*pRequest));
					break;
				}
				case EResolverRequest::SERVICE:
				{
					processService(static_cast<ServiceRequest&>(*pRequest));
					break;
				}
				case EResolverRequest::ADDRESS:
				{
					processAddress(static_cast<AddressRequest&>(*pRequest));
					break;
				}
				case EResolverRequest::PORT:
				{
					processPort(static_cast<PortRequest&>(*pRequest));
					break;
				}
			}

			gApp->getEventSystem()->dispatch<ResolverEvent>(std::move(pRequest));
		}
	}

	void processHostname(HostnameRequest & request)
	{
		AddressPack & pack = request.getAddressPack();

		pack = Resolver::PlatformResolveHostname(request.getHostname());

		if (gLog->isMsgEnabled(Log::INFO))
		{
			for (size_t i = 0; i < pack.getSize(); i++)
			{
				const IAddress & address = pack[i];

				gLog->info("[Resolver] Hostname resolved: '%s' --> %s %s",
				  request.getHostname().c_str(),
				  address.getTypeName().c_str(),
				  address.toString().c_str()
				);
			}
		}
	}

	void processService(ServiceRequest & request)
	{
		PortPack & pack = request.getPortPack();

		pack = Resolver::PlatformResolveService(request.getService(), request.getPortType());

		if (gLog->isMsgEnabled(Log::INFO))
		{
			for (size_t i = 0; i < pack.getSize(); i++)
			{
				const Port & port = pack[i];

				gLog->info("[Resolver] Service resolved: '%s' --> %s %hu",
				  request.getService().c_str(),
				  port.getTypeName().c_str(),
				  port.getNumber()
				);
			}
		}
	}

	void processAddress(AddressRequest & request)
	{
		const IAddress & address = request.getAddressData().getAddress();
		ResolvedAddress & resolved = request.getResolvedData();

		if (m_isAddressHostnameEnabled)
		{
			resolved.hostname = Resolver::PlatformResolveAddress(address);
		}

		if (gApp->hasGeoIP())
		{
			resolved.country = gApp->getGeoIP()->queryCountry(address);
			resolved.asn = gApp->getGeoIP()->queryASN(address);
		}

		if (gLog->isMsgEnabled(Log::INFO))
		{
			gLog->info("[Resolver] Address resolved: %s %s --> '%s' | country: %s | %s %s",
			  address.getTypeName().c_str(),
			  request.getAddressData().getNumericString().c_str(),
			  resolved.hostname.c_str(),
			  resolved.country.getCodeString().c_str(),
			  resolved.asn.getString().c_str(),
			  resolved.asn.getOrgName().c_str()
			);
		}
	}

	void processPort(PortRequest & request)
	{
		const Port & port = request.getPortData().getPort();
		ResolvedPort & resolved = request.getResolvedData();

		if (m_isPortServiceEnabled)
		{
			resolved.service = Resolver::PlatformResolvePort(port);
		}

		if (!resolved.service.empty() && gLog->isMsgEnabled(Log::INFO))
		{
			gLog->info("[Resolver] Port resolved: %s %hu --> '%s'",
			  port.getTypeName().c_str(),
			  port.getNumber(),
			  resolved.service.c_str()
			);
		}
	}

public:
	Impl()
	: m_requestQueue(),
	  m_resolverThread(),
	  m_isRunning(),
	  m_isAddressHostnameEnabled(true),
	  m_isPortServiceEnabled(true)
	{
		if (gCmdLine->hasArg("no-hostname"))
		{
			m_isAddressHostnameEnabled = false;
			gLog->notice("[Resolver] Address hostname resolving disabled by command line");
		}

		if (gCmdLine->hasArg("no-servname"))
		{
			m_isPortServiceEnabled = false;
			gLog->notice("[Resolver] Port service name resolving disabled by command line");
		}

		m_isRunning = true;

		auto ResolverThreadFunction = [this]() -> void
		{
			resolverLoop();
		};

		// start resolver thread
		m_resolverThread = Thread("Resolver", ResolverThreadFunction);

		gApp->getEventSystem()->registerCallback<ResolverEvent>(this);
	}

	~Impl()
	{
		m_isRunning = false;

		gApp->getEventSystem()->removeCallback<ResolverEvent>(this);

		// stop resolver thread
		m_requestQueue.enqueue(nullptr);  // wake resolver thread
		m_resolverThread.join();
	}

	void onEvent(const ResolverEvent & event) override
	{
		if (event.isEmpty())
		{
			return;
		}

		IResolverRequest & genericRequest = const_cast<IResolverRequest&>(event.getRequest());

		switch (genericRequest.getType())
		{
			case EResolverRequest::HOSTNAME:
			{
				HostnameRequest & request = static_cast<HostnameRequest&>(genericRequest);
				Resolver::CallbackHostname & callback = request.getCallback();

				callback(
				  request.getHostname(),
				  request.getAddressPack(),
				  request.getCallbackParam()
				);

				break;
			}
			case EResolverRequest::SERVICE:
			{
				ServiceRequest & request = static_cast<ServiceRequest&>(genericRequest);
				Resolver::CallbackService & callback = request.getCallback();

				callback(
				  request.getService(),
				  request.getPortType(),
				  request.getPortPack(),
				  request.getCallbackParam()
				);

				break;
			}
			case EResolverRequest::ADDRESS:
			{
				AddressRequest & request = static_cast<AddressRequest&>(genericRequest);
				Resolver::CallbackAddress & callback = request.getCallback();

				callback(
				  request.getAddressData(),
				  request.getResolvedData(),
				  request.getCallbackParam()
				);

				break;
			}
			case EResolverRequest::PORT:
			{
				PortRequest & request = static_cast<PortRequest&>(genericRequest);
				Resolver::CallbackPort & callback = request.getCallback();

				callback(
				  request.getPortData(),
				  request.getResolvedData(),
				  request.getCallbackParam()
				);

				break;
			}
		}
	}

	bool isAddressHostnameEnabled() const
	{
		return m_isAddressHostnameEnabled;
	}

	bool isPortServiceEnabled() const
	{
		return m_isPortServiceEnabled;
	}

	void pushRequest(std::unique_ptr<IResolverRequest> && request)
	{
		m_requestQueue.enqueue(std::move(request));
	}
};

Resolver::Resolver()
: m_impl(std::make_unique<Impl>())
{
}

Resolver::~Resolver()
{
}

bool Resolver::isAddressHostnameEnabled() const
{
	return m_impl->isAddressHostnameEnabled();
}

bool Resolver::isPortServiceEnabled() const
{
	return m_impl->isPortServiceEnabled();
}

void Resolver::resolveHostname(std::string hostname, const CallbackHostname & callback, void *param)
{
	m_impl->pushRequest(std::make_unique<HostnameRequest>(std::move(hostname), callback, param));
}

void Resolver::resolveService(std::string service, EPortType type, const CallbackService & callback, void *param)
{
	m_impl->pushRequest(std::make_unique<ServiceRequest>(std::move(service), type, callback, param));
}

void Resolver::resolveAddress(AddressData & address, const CallbackAddress & callback, void *param)
{
	m_impl->pushRequest(std::make_unique<AddressRequest>(address, callback, param));
}

void Resolver::resolvePort(PortData & port, const CallbackPort & callback, void *param)
{
	m_impl->pushRequest(std::make_unique<PortRequest>(port, callback, param));
}
