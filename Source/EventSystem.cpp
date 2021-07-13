/**
 * @file
 * @brief Implementation of EventSystem class.
 */

#include <unordered_map>

#include "EventSystem.hpp"
#include "Events.hpp"
#include "Log.hpp"

#include "concurrentqueue/blockingconcurrentqueue.h"

class EventCallbackData
{
	void *m_pCallback;
	void *m_pExecutor;

public:
	EventCallbackData(void *pCallback, void *pExecutor)
	: m_pCallback(pCallback),
	  m_pExecutor(pExecutor)
	{
	}

	void *getCallback() const
	{
		return m_pCallback;
	}

	void *getExecutor() const
	{
		return m_pExecutor;
	}
};

class EventSystem::Impl
{
	moodycamel::BlockingConcurrentQueue<EventWrapper> m_eventQueue;
	std::unordered_multimap<int, EventCallbackData> m_callbackMap;
	bool m_isRunning;

public:
	Impl()
	: m_eventQueue(),
	  m_callbackMap(),
	  m_isRunning()
	{
	}

	void pushEvent(EventWrapper && eventWrapper)
	{
		m_eventQueue.enqueue(std::move(eventWrapper));
	}

	void addCallback(void *pCallback, void *pExecutor, int eventID)
	{
		m_callbackMap.emplace(eventID, EventCallbackData(pCallback, pExecutor));

		KString eventName = EGlobalEventID::ToString(eventID);
		gLog->debug("[EventSystem] Registered callback of event %d (%s)", eventID, eventName.c_str());
	}

	void delCallback(void *pCallback, int eventID)
	{
		bool isRemoved = false;
		const auto callbackRange = m_callbackMap.equal_range(eventID);
		for (auto it = callbackRange.first; it != callbackRange.second;)
		{
			if (it->second.getCallback() == pCallback)
			{
				it = m_callbackMap.erase(it);
				isRemoved = true;
			}
			else
			{
				++it;
			}
		}

		if (isRemoved)
		{
			KString eventName = EGlobalEventID::ToString(eventID);
			gLog->debug("[EventSystem] Removed callback of event %d (%s)", eventID, eventName.c_str());
		}
	}

	void run()
	{
		m_isRunning = true;
		moodycamel::ConsumerToken token(m_eventQueue);

		gLog->debug("[EventSystem] Dispatcher started");

		while (m_isRunning)
		{
			EventWrapper eventWrapper;
			// wait for event
			m_eventQueue.wait_dequeue(token, eventWrapper);

			const int eventID = eventWrapper.getEventID();
			const auto callbackRange = m_callbackMap.equal_range(eventID);
			// execute all callbacks registered for the event
			for (auto it = callbackRange.first; it != callbackRange.second; ++it)
			{
				const EventCallbackData & data = it->second;
				ExecutorFunction executor = reinterpret_cast<ExecutorFunction>(data.getExecutor());

				executor(data.getCallback(), eventWrapper);
			}
		}

		gLog->debug("[EventSystem] Dispatcher stopped");
	}

	void stop()
	{
		m_isRunning = false;
	}
};

EventSystem::EventSystem()
: m_impl(std::make_unique<Impl>())
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::run()
{
	m_impl->run();
}

void EventSystem::stop()
{
	m_impl->stop();
}

void EventSystem::pushEvent(EventWrapper && eventWrapper)
{
	m_impl->pushEvent(std::move(eventWrapper));
}

void EventSystem::addCallback(void *pCallback, void *pExecutor, int eventID)
{
	m_impl->addCallback(pCallback, pExecutor, eventID);
}

void EventSystem::delCallback(void *pCallback, int eventID)
{
	m_impl->delCallback(pCallback, eventID);
}
