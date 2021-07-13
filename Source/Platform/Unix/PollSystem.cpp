/**
 * @file
 * @brief Implementation of PollSystem class for Unix platform.
 */

#include <unordered_map>

#include "PollSystem.hpp"
#include "PollHandle.hpp"
#include "SelfPipe.hpp"
#include "Thread.hpp"
#include "App.hpp"
#include "Log.hpp"

#include "readerwriterqueue/readerwriterqueue.h"

namespace ctp
{
	class PollRequest
	{
	public:
		enum EType
		{
			NONE, ADD, RESET, REMOVE
		};

	private:
		EType m_type;
		int m_fd;
		int m_flags;

	public:
		PollRequest(EType type = NONE, int fd = -1, int flags = 0)
		: m_type(type),
		  m_fd(fd),
		  m_flags(flags)
		{
		}

		bool isEmpty() const
		{
			return m_type == NONE;
		}

		EType getType() const
		{
			return m_type;
		}

		int getDescriptor() const
		{
			return m_fd;
		}

		int getFlags() const
		{
			return m_flags;
		}
	};

	class PollCallbackData
	{
		PollSystem::Callback m_callback;
		void *m_param;

	public:
		PollCallbackData(const PollSystem::Callback & callback, void *param)
		: m_callback(callback),
		  m_param(param)
		{
		}

		const PollSystem::Callback & getCallback() const
		{
			return m_callback;
		}

		void *getParam() const
		{
			return m_param;
		}
	};

	class PollSystem::Impl final : public IEventCallback<PollEvent>
	{
		std::unordered_map<int, PollCallbackData> m_callbackMap;
		moodycamel::ReaderWriterQueue<PollRequest> m_requestQueue;
		Thread m_pollThread;
		SelfPipe m_pipe;
		bool m_isRunning;

		void pollLoop()  // executed by poll thread
		{
			PollHandle pollHandle;

			const int pipeFD = m_pipe.getReadFD();
			pollHandle.add(pipeFD, EPollFlags::INPUT);

			while (m_isRunning)
			{
				pollHandle.wait();

				bool checkRequestQueue = false;
				PollEvent event = pollHandle.getNextEvent();
				while (!event.isEmpty())
				{
					if (event.getDescriptor() == pipeFD)
					{
						m_pipe.clear();
						pollHandle.reset(pipeFD, EPollFlags::INPUT);
						checkRequestQueue = true;
					}
					else
					{
						gApp->getEventSystem()->dispatch<PollEvent>(std::move(event));
					}

					event = pollHandle.getNextEvent();
				}

				if (!checkRequestQueue)
				{
					continue;
				}

				PollRequest request;
				while (m_requestQueue.try_dequeue(request))
				{
					switch (request.getType())
					{
						case PollRequest::RESET:
						{
							pollHandle.reset(request.getDescriptor(), request.getFlags());
							break;
						}
						case PollRequest::ADD:
						{
							pollHandle.add(request.getDescriptor(), request.getFlags());
							gLog->debug("[PollSystem] File descriptor %d registered (flags: 0x%X)",
							  request.getDescriptor(),
							  request.getFlags()
							);
							break;
						}
						case PollRequest::REMOVE:
						{
							pollHandle.remove(request.getDescriptor());
							gLog->debug("[PollSystem] File descriptor %d removed",
							  request.getDescriptor()
							);
							break;
						}
						case PollRequest::NONE:
						{
							break;
						}
					}
				}
			}
		}

		void wakePollThread()
		{
			const char *something = "A";
			m_pipe.writeData(something, 1);
		}

		void pushRequest(PollRequest::EType type, int fd, int flags = 0)
		{
			m_requestQueue.emplace(type, fd, flags);
			wakePollThread();
		}

	public:
		Impl()
		: m_callbackMap(),
		  m_requestQueue(),
		  m_pollThread(),
		  m_pipe(),
		  m_isRunning()
		{
			m_isRunning = true;

			auto PollThreadFunction = [this]() -> void
			{
				pollLoop();
			};

			// start poll thread
			m_pollThread = Thread("Poll", PollThreadFunction);

			gApp->getEventSystem()->registerCallback<PollEvent>(this);
		}

		~Impl()
		{
			m_isRunning = false;

			gApp->getEventSystem()->removeCallback<PollEvent>(this);

			// stop poll thread
			wakePollThread();
			m_pollThread.join();
		}

		void onEvent(const PollEvent & event) override
		{
			auto it = m_callbackMap.find(event.getDescriptor());
			if (it != m_callbackMap.end())
			{
				const PollCallbackData & data = it->second;
				const Callback & callback = data.getCallback();

				const int fd = event.getDescriptor();

				if (event.hasError())
				{
					gLog->debug("[PollSystem] Error occurred on file descriptor %d", fd);
				}

				if (event.hasHangUp())
				{
					gLog->debug("[PollSystem] Hang up occurred on file descriptor %d", fd);
				}

				if (event.hasInvalidFD())
				{
					gLog->error("[PollSystem] Invalid file descriptor %d", fd);
				}

				callback(event.getFlags(), data.getParam());
			}
		}

		void addFD(int fd, int flags, const Callback & callback, void *param)
		{
			auto it = m_callbackMap.find(fd);
			if (it != m_callbackMap.end())
			{
				gLog->error("[PollSystem] File descriptor %d is already registered", fd);
			}
			else
			{
				m_callbackMap.emplace(fd, PollCallbackData(callback, param));
				pushRequest(PollRequest::ADD, fd, flags);
			}
		}

		void resetFD(int fd, int flags)
		{
			pushRequest(PollRequest::RESET, fd, flags);
		}

		void removeFD(int fd)
		{
			if (m_callbackMap.erase(fd) > 0)
			{
				pushRequest(PollRequest::REMOVE, fd);
			}
			else
			{
				gLog->error("[PollSystem] File descriptor %d is not registered", fd);
			}
		}
	};

	PollSystem::PollSystem()
	: m_impl(std::make_unique<Impl>())
	{
	}

	PollSystem::~PollSystem()
	{
	}

	void PollSystem::addFD(int fd, int flags, const Callback & callback, void *param)
	{
		m_impl->addFD(fd, flags, callback, param);
	}

	void PollSystem::resetFD(int fd, int flags)
	{
		m_impl->resetFD(fd, flags);
	}

	void PollSystem::removeFD(int fd)
	{
		m_impl->removeFD(fd);
	}
}
