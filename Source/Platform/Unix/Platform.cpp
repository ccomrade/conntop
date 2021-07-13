/**
 * @file
 * @brief Implementation of Platform class for Unix platform.
 */

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>  // gettimeofday
#include <cerrno>
#include <cstdio>  // std::sscanf
#include <cstring>  // std::strchr
#include <fstream>
#include <system_error>

#include "Platform.hpp"
#include "Thread.hpp"
#include "Events.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "Util.hpp"
#include "conntop_config.h"

#ifdef CONNTOP_UI_CURSES
#include "UI_Curses/CursesEvent.hpp"
#endif

static const int UPDATE_TIMER_SIGNAL = SIGRTMIN + 0;

#ifdef CONNTOP_UI_CURSES
static void EmptySignalHandler(int)
{
}
#endif

class Platform::Impl
{
	bool m_isRunning;
	Thread m_signalThread;
	sigset_t m_signalMask;
	timer_t m_updateTimer;

	void signalLoop()  // executed by signal thread
	{
		while (m_isRunning)
		{
			siginfo_t signal;
			// this should always succeed
			if (sigwaitinfo(&m_signalMask, &signal) < 0)
			{
				// well, there is one error that might happen, but it's not a real error
				if (errno == EINTR)
				{
					// waiting was interrupted by some unblocked signal
					continue;
				}

				throw std::system_error(errno, std::system_category(), "Waiting for signal failed");
			}

			if (signal.si_signo == UPDATE_TIMER_SIGNAL)
			{
				// PID 0 means the signal was sent by kernel
				if (signal.si_pid == 0 && signal.si_value.sival_ptr == &m_updateTimer)
				{
					gLog->debug("[Platform] Signal: UPDATE_TIMER_SIGNAL");
					gApp->getEventSystem()->dispatch<UpdateEvent>();
				}
			}
			else
			{
				handleSignal(signal.si_signo);
			}
		}
	}

	void handleSignal(int signalNumber)
	{
		switch (signalNumber)
		{
			case SIGHUP:
			{
				gLog->debug("[Platform] Signal: SIGHUP");
				gApp->quit();
				break;
			}
			case SIGINT:
			{
				// this signal is also used to wake up signal thread (see stop function)
				if (m_isRunning)
				{
					gLog->debug("[Platform] Signal: SIGINT");
					gApp->quit();
				}
				break;
			}
			case SIGTERM:
			{
				gLog->debug("[Platform] Signal: SIGTERM");
				gApp->quit();
				break;
			}
		#ifdef CONNTOP_UI_CURSES
			case SIGWINCH:
			{
				gLog->debug("[Platform] Signal: SIGWINCH");
				gApp->getEventSystem()->dispatch<CursesEvent>(CursesEvent::TERMINAL_RESIZED);
				break;
			}
		#endif
			case SIGUSR1:
			{
				gLog->debug("[Platform] Signal: SIGUSR1");
				Util::LogMemoryUsage(true);
				break;
			}
			default:
			{
				gLog->warning("[Platform] Unknown signal %d", signalNumber);
			}
		}
	}

public:
	Impl()
	: m_isRunning(false),
	  m_signalThread(),
	  m_signalMask(),
	  m_updateTimer()
	{
		sigemptyset(&m_signalMask);

		sigaddset(&m_signalMask, SIGHUP);
		sigaddset(&m_signalMask, SIGINT);
		sigaddset(&m_signalMask, SIGTERM);
		sigaddset(&m_signalMask, SIGTSTP);
		sigaddset(&m_signalMask, SIGUSR1);

		sigaddset(&m_signalMask, UPDATE_TIMER_SIGNAL);

	#ifdef CONNTOP_UI_CURSES
		// SIGWINCH signal is not standardized but exists in all modern POSIX systems
		sigaddset(&m_signalMask, SIGWINCH);
	#endif

		if (int errNum = pthread_sigmask(SIG_BLOCK, &m_signalMask, nullptr))
		{
			throw std::system_error(errNum, std::system_category(), "Unable to block signals");
		}

	#ifdef CONNTOP_UI_CURSES
		// prevent curses library from installing its own unsafe signal handlers
		struct sigaction empty;
		empty.sa_handler = EmptySignalHandler;
		empty.sa_flags = 0;
		sigemptyset(&empty.sa_mask);
		sigaction(SIGHUP, &empty, nullptr);
		sigaction(SIGINT, &empty, nullptr);
		sigaction(SIGTERM, &empty, nullptr);
		sigaction(SIGTSTP, &empty, nullptr);
		sigaction(SIGWINCH, &empty, nullptr);
	#endif
	}

	~Impl()
	{
		if (m_isRunning)
		{
			stop();
		}
	}

	bool isRunning() const
	{
		return m_isRunning;
	}

	void start()
	{
		m_isRunning = true;

		sigevent updateTimerEvent;
		updateTimerEvent.sigev_notify = SIGEV_SIGNAL;
		updateTimerEvent.sigev_signo = UPDATE_TIMER_SIGNAL;
		updateTimerEvent.sigev_value.sival_ptr = &m_updateTimer;

		if (timer_create(CLOCK_REALTIME, &updateTimerEvent, &m_updateTimer) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to create update timer");
		}

		itimerspec updateTimerSpec;
		updateTimerSpec.it_interval.tv_sec  = 1;
		updateTimerSpec.it_interval.tv_nsec = 0;
		updateTimerSpec.it_value.tv_sec     = updateTimerSpec.it_interval.tv_sec;
		updateTimerSpec.it_value.tv_nsec    = updateTimerSpec.it_interval.tv_nsec;

		if (timer_settime(m_updateTimer, 0, &updateTimerSpec, nullptr) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to start update timer");
		}

		auto SignalThreadFunction = [this]() -> void
		{
			signalLoop();
		};

		// start signal thread
		m_signalThread = Thread("Signal", SignalThreadFunction);
	}

	void stop()
	{
		m_isRunning = false;

		timer_delete(m_updateTimer);

		// stop signal thread
		pthread_kill(m_signalThread.getNativeHandle(), SIGINT);
		m_signalThread.join();
	}
};

Platform::Platform()
: m_impl(std::make_unique<Impl>())
{
}

Platform::~Platform()
{
}

KString Platform::getImplementationName() const
{
	return "Unix";
}

KString Platform::getSystemName() const
{
#ifdef CONNTOP_PLATFORM_LINUX
	return "Linux";
#else
	return "unknown";
#endif
}

void Platform::start()
{
	if (!m_impl->isRunning())
	{
		m_impl->start();
	}
}

void Platform::stop()
{
	if (m_impl->isRunning())
	{
		m_impl->stop();
	}
}

std::string Platform::getCurrentHostName()
{
	char buffer[256];
	if (gethostname(buffer, sizeof buffer) < 0)
	{
		if (gLog)
		{
			gLog->error("[Platform] gethostname failed: %s", Util::ErrnoToString().c_str());
		}
		return std::string();
	}

	// hostname may not be null terminated if the buffer is not large enough
	buffer[(sizeof buffer)-1] = '\0';

	return std::string(buffer);
}

UnixTime Platform::getCurrentUnixTime()
{
	timeval time;
	if (gettimeofday(&time, nullptr) < 0)
	{
		if (gLog)
		{
			gLog->error("[Platform] gettimeofday failed: %s", Util::ErrnoToString().c_str());
		}
		return UnixTime();
	}

	return UnixTime(time.tv_sec, time.tv_usec / 1000);
}

DateTime Platform::getCurrentDateTime(DateTime::EType type)
{
	UnixTime unixTime = getCurrentUnixTime();

	if (type == DateTime::UTC)
	{
		return Util::UnixTimeToDateTimeUTC(unixTime);
	}
	else
	{
		return Util::UnixTimeToDateTimeLocal(unixTime);
	}
}

PlatformProcessMemoryUsage Platform::getProcessMemoryUsage()
{
	enum
	{
		MEM_TOTAL,
		MEM_PEAK_TOTAL,
		MEM_ANONYMOUS,
		MEM_MAPPED_FILES,
		MEM_SHARED,

		_COUNT
	};

	long usage[_COUNT];

	// initialize usage array
	for (unsigned int i = 0; i < _COUNT; i++)
	{
		usage[i] = -1;
	}

#ifdef CONNTOP_PLATFORM_LINUX
	constexpr KString VALUE_NAME_MAP[_COUNT] = {
		[MEM_TOTAL]        = "VmRSS",
		[MEM_PEAK_TOTAL]   = "VmHWM",
		[MEM_ANONYMOUS]    = "RssAnon",
		[MEM_MAPPED_FILES] = "RssFile",
		[MEM_SHARED]       = "RssShmem"
	};

	std::ifstream statusFile("/proc/self/status");
	if (statusFile.is_open())
	{
		char buffer[256];
		while (statusFile.good())
		{
			statusFile.getline(buffer, sizeof buffer);

			KString name;
			KString value;
			// split line to name and value
			if (char *delim = std::strchr(buffer, ':'))
			{
				(*delim) = '\0';
				name = buffer;
				value = delim+1;
			}
			else
			{
				// no delimiter found
				continue;
			}

			bool haveAllValues = true;
			for (unsigned int i = 0; i < _COUNT; i++)
			{
				if (usage[i] < 0)
				{
					haveAllValues = false;
					if (VALUE_NAME_MAP[i] == name)
					{
						std::sscanf(value.c_str(), " %ld kB", &usage[i]);
						break;
					}
				}
			}

			if (haveAllValues)
			{
				break;
			}
		}
	}
#endif

	return PlatformProcessMemoryUsage(
	  usage[MEM_TOTAL],
	  usage[MEM_PEAK_TOTAL],
	  usage[MEM_ANONYMOUS],
	  usage[MEM_MAPPED_FILES],
	  usage[MEM_SHARED]
	);
}
