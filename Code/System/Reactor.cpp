#include <sys/epoll.h>
#include <cerrno>
#include <array>
#include <stdexcept>
#include <system_error>

#include "Reactor.h"
#include "Log.h"

void Reactor::Init()
{
	m_epoll = Handle(epoll_create1(EPOLL_CLOEXEC));
	if (!m_epoll)
	{
		throw std::system_error(errno, std::system_category(), "Failed to create epoll");
	}

	m_isRunning = true;

	LOG_DEBUG(Format("[Reactor] Created epoll instance on file descriptor %d", m_epoll.GetFileDescriptor()));
}

Reactor::Handler& Reactor::GetHandler(int fd)
{
	const auto index = static_cast<unsigned int>(fd);

	if (index >= m_handlers.size())
	{
		m_handlers.resize(index + 1);
	}

	return m_handlers[index];
}

void Reactor::CommitAllChanges()
{
	if (m_isCommitNeeded)
	{
		int fd = 0;

		for (Handler& handler : m_handlers)
		{
			Commit(fd++, handler);
		}

		m_isCommitNeeded = false;
	}
}

void Reactor::Commit(int fd, Handler& handler)
{
	const bool hasRead = static_cast<bool>(handler.onRead);
	const bool hasWrite = static_cast<bool>(handler.onWrite);
	const bool hasError = static_cast<bool>(handler.onError);

	const bool nothingEnabled = !hasRead && !hasWrite && !hasError;

	const auto buildConfig = [&fd](bool enableRead, bool enableWrite) -> epoll_event
	{
		epoll_event config = {};

		if (enableRead)
			config.events |= EPOLLIN;

		if (enableWrite)
			config.events |= EPOLLOUT;

		config.data.fd = fd;

		return config;
	};

	epoll_event oldConfig = buildConfig(handler.isReadEnabled, handler.isWriteEnabled);
	epoll_event newConfig = buildConfig(hasRead, hasWrite);

	if (!handler.isRegistered)
	{
		if (nothingEnabled)
		{
			return;
		}

		LOG_DEBUG(Format("[Reactor] EPOLL_CTL_ADD | fd: %d | events: %u", fd, newConfig.events));

		if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_ADD, fd, &newConfig) < 0)
		{
			throw std::system_error(errno, std::system_category(),
			                        "Failed to register file descriptor " + std::to_string(fd) + " in epoll");
		}

		handler.isRegistered = true;
	}
	else if (nothingEnabled)
	{
		LOG_DEBUG(Format("[Reactor] EPOLL_CTL_DEL | fd: %d", fd));

		if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_DEL, fd, nullptr) < 0)
		{
			throw std::system_error(errno, std::system_category(),
			                        "Failed to delete file descriptor " + std::to_string(fd) + " in epoll");
		}

		handler.isRegistered = false;
	}
	else if (oldConfig.events != newConfig.events)
	{
		LOG_DEBUG(Format("[Reactor] EPOLL_CTL_MOD | fd: %d | events: %u -> %u", fd, oldConfig.events, newConfig.events));

		if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_MOD, fd, &newConfig) < 0)
		{
			throw std::system_error(errno, std::system_category(),
			                        "Failed to modify file descriptor " + std::to_string(fd) + " in epoll");
		}
	}

	handler.isReadEnabled = hasRead;
	handler.isWriteEnabled = hasWrite;
	handler.isErrorEnabled = hasError;
}

void Reactor::DispatchEvent(int fd, bool isRead, bool isWrite, bool isError)
{
	Handler& handler = GetHandler(fd);

	if (isError)
	{
		if (handler.onError)
		{
			ExecuteCallback(handler.onError);
		}
		else
		{
			throw std::runtime_error("Unhandled error event on file descriptor " + std::to_string(fd));
		}
	}

	if (isRead)
	{
		ExecuteCallback(handler.onRead);
	}

	if (isWrite)
	{
		ExecuteCallback(handler.onWrite);
	}
}

void Reactor::ExecuteCallback(Callback& callback)
{
	if (callback)
	{
		const CallbackResult result = callback();

		if (result == CallbackResult::STOP)
		{
			callback = {};
			m_isCommitNeeded = true;
		}
	}
}

void Reactor::EventLoop()
{
	std::array<epoll_event, 64> buffer = {};

	while (m_isRunning)
	{
		CommitAllChanges();

		const int eventCount = epoll_wait(m_epoll.GetFileDescriptor(), buffer.data(), buffer.size(), -1);
		if (eventCount < 0)
		{
			const int code = errno;

			if (code == EINTR)
			{
				LOG_DEBUG("[Reactor] Waiting interrupted by a signal");
			}
			else
			{
				throw std::system_error(code, std::system_category(), "Failed to wait on epoll");
			}
		}

		for (int i = 0; i < eventCount; i++)
		{
			const auto flags = buffer[i].events;

			const bool isRead = (flags & EPOLLIN);
			const bool isWrite = (flags & EPOLLOUT);
			const bool isError = (flags & EPOLLERR) || (flags & EPOLLHUP);

			const int fd = buffer[i].data.fd;

			DispatchEvent(fd, isRead, isWrite, isError);
		}
	}
}

void Reactor::StopEventLoop()
{
	m_isRunning = false;
}

void Reactor::AttachReadHandler(const Handle& handle, Callback&& callback)
{
	AttachReadHandler(handle.GetFileDescriptor(), std::move(callback));
}

void Reactor::AttachWriteHandler(const Handle& handle, Callback&& callback)
{
	AttachWriteHandler(handle.GetFileDescriptor(), std::move(callback));
}

void Reactor::AttachErrorHandler(const Handle& handle, Callback&& callback)
{
	AttachErrorHandler(handle.GetFileDescriptor(), std::move(callback));
}

void Reactor::Detach(const Handle& handle)
{
	Detach(handle.GetFileDescriptor());
}

void Reactor::AttachReadHandler(int fd, Callback&& callback)
{
	if (fd < 0)
	{
		return;
	}

	Handler& handler = GetHandler(fd);

	const bool hadReadBefore = static_cast<bool>(handler.onRead);
	const bool hasReadNow = static_cast<bool>(callback);

	handler.onRead = std::move(callback);

	if (hadReadBefore != hasReadNow)
	{
		m_isCommitNeeded = true;
	}
}

void Reactor::AttachWriteHandler(int fd, Callback&& callback)
{
	if (fd < 0)
	{
		return;
	}

	Handler& handler = GetHandler(fd);

	const bool hadWriteBefore = static_cast<bool>(handler.onWrite);
	const bool hasWriteNow = static_cast<bool>(callback);

	handler.onWrite = std::move(callback);

	if (hadWriteBefore != hasWriteNow)
	{
		m_isCommitNeeded = true;
	}
}

void Reactor::AttachErrorHandler(int fd, Callback&& callback)
{
	if (fd < 0)
	{
		return;
	}

	Handler& handler = GetHandler(fd);

	const bool hadErrorBefore = static_cast<bool>(handler.onWrite);
	const bool hasErrorNow = static_cast<bool>(callback);

	handler.onError = std::move(callback);

	if (hadErrorBefore != hasErrorNow)
	{
		m_isCommitNeeded = true;
	}
}

void Reactor::Detach(int fd)
{
	if (fd < 0)
	{
		return;
	}

	Handler& handler = GetHandler(fd);

	handler.onRead = {};
	handler.onWrite = {};
	handler.onError = {};

	if (handler.isReadEnabled || handler.isWriteEnabled || handler.isErrorEnabled)
	{
		m_isCommitNeeded = true;
	}
}
