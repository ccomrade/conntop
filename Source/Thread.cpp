/**
 * @file
 * @brief Implementation of Thread class.
 */

#include "Thread.hpp"
#include "Exception.hpp"
#include "Log.hpp"
#include "App.hpp"

namespace ctp
{
	static thread_local std::string g_threadName;

	const KString Thread::DEFAULT_NAME = "<unnamed>";

	Thread::Thread(std::string threadName, const Function & threadFunction)
	: m_name(std::move(threadName)),
	  m_thread()
	{
		if (m_name.empty())
		{
			m_name = DEFAULT_NAME;
		}

		m_thread = std::thread(Run, m_name, threadFunction);  // may throw std::system_error
	}

	void Thread::join()
	{
		if (isJoinable())
		{
			gLog->info("[Thread] Waiting for %s thread...", m_name.c_str());
			m_thread.join();
			gLog->info("[Thread] %s thread stopped", m_name.c_str());
		}
	}

	KString Thread::GetCurrentThreadName()
	{
		if (g_threadName.empty())
		{
			return DEFAULT_NAME;
		}
		else
		{
			return g_threadName;
		}
	}

	void Thread::SetCurrentThreadName(std::string name)
	{
		g_threadName = std::move(name);
		PlatformCurrentThreadSetName(g_threadName);
	}

	void Thread::Run(std::string threadName, Function threadFunction)
	{
		SetCurrentThreadName(std::move(threadName));

		const KString name = GetCurrentThreadName();
		gLog->info("[Thread] %s thread started", name.c_str());

		try
		{
			threadFunction();
		}
		catch (const Exception & e)
		{
			KString origin = (e.hasOrigin()) ? e.getOrigin() : "?";
			gLog->info("[Thread] Exception in %s thread: [%s] %s", name.c_str(), origin.c_str(), e.what());

			gApp->fatalError(e.getString(), e.getOrigin(), false);
		}
	}
}
