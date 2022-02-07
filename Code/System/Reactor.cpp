#include <sys/epoll.h>
#include <errno.h>
#include <array>
#include <stdexcept>
#include <system_error>

#include "Reactor.h"

namespace
{
	epoll_event CreateConfig(int fd, ReactorEvents events)
	{
		epoll_event config = {};

		switch (events)
		{
			case ReactorEvents::NONE_EXCEPT_ERRORS:
				config.events = 0;
				break;
			case ReactorEvents::READ_ONLY:
				config.events = EPOLLIN;
				break;
			case ReactorEvents::WRITE_ONLY:
				config.events = EPOLLOUT;
				break;
			case ReactorEvents::READ_WRITE:
				config.events = EPOLLIN | EPOLLOUT;
				break;
		}

		config.data.fd = fd;

		return config;
	}

	bool IsReadEnabled(ReactorEvents events)
	{
		switch (events)
		{
			case ReactorEvents::READ_ONLY:
			case ReactorEvents::READ_WRITE:
				return true;
			default:
				return false;
		}
	}

	bool IsWriteEnabled(ReactorEvents events)
	{
		switch (events)
		{
			case ReactorEvents::WRITE_ONLY:
			case ReactorEvents::READ_WRITE:
				return true;
			default:
				return false;
		}
	}

	ReactorEvents DisableRead(ReactorEvents events)
	{
		switch (events)
		{
			case ReactorEvents::READ_ONLY:
				return ReactorEvents::NONE_EXCEPT_ERRORS;
			case ReactorEvents::READ_WRITE:
				return ReactorEvents::WRITE_ONLY;
			default:
				return events;
		}
	}

	ReactorEvents DisableWrite(ReactorEvents events)
	{
		switch (events)
		{
			case ReactorEvents::WRITE_ONLY:
				return ReactorEvents::NONE_EXCEPT_ERRORS;
			case ReactorEvents::READ_WRITE:
				return ReactorEvents::READ_ONLY;
			default:
				return events;
		}
	}
}

void Reactor::Init()
{
	m_epoll = Handle(epoll_create1(EPOLL_CLOEXEC));
	if (!m_epoll)
	{
		throw std::system_error(errno, std::system_category(), "Failed to create epoll");
	}

	m_running = true;
}

void Reactor::Attach(const Handle& handle, ReactorHandler&& handler)
{
	const int fd = handle.GetFileDescriptor();

	Attach(fd, std::move(handler));
}

void Reactor::Modify(const Handle& handle, ReactorEvents events)
{
	const int fd = handle.GetFileDescriptor();

	Modify(fd, events);
}

void Reactor::Detach(const Handle& handle)
{
	const int fd = handle.GetFileDescriptor();

	Detach(fd);
}

void Reactor::Attach(int fd, ReactorHandler&& handler)
{
	if (fd >= 0 && static_cast<size_t>(fd) >= m_handlers.size())
	{
		m_handlers.resize(fd + 1);
	}

	epoll_event config = CreateConfig(fd, handler.events);

	if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_ADD, fd, &config) < 0)
	{
		throw std::system_error(errno, std::system_category(),
		                        "Failed to attach file descriptor " + std::to_string(fd) + " to epoll");
	}

	m_handlers[fd] = std::move(handler);
}

void Reactor::Modify(int fd, ReactorEvents events)
{
	if (fd < 0 && static_cast<size_t>(fd) >= m_handlers.size())
	{
		// invalid file descriptor
		return;
	}

	ReactorHandler& handler = m_handlers[fd];

	if (handler.events == events)
	{
		// nothing to do
		return;
	}

	epoll_event config = CreateConfig(fd, events);

	if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_MOD, fd, &config) < 0)
	{
		throw std::system_error(errno, std::system_category(),
		                        "Failed to modify file descriptor " + std::to_string(fd) + " in epoll");
	}

	handler.events = events;
}

void Reactor::Detach(int fd)
{
	if (fd < 0 && static_cast<size_t>(fd) >= m_handlers.size())
	{
		// invalid file descriptor
		return;
	}

	m_handlers[fd] = ReactorHandler();

	if (epoll_ctl(m_epoll.GetFileDescriptor(), EPOLL_CTL_DEL, fd, nullptr) < 0)
	{
		throw std::system_error(errno, std::system_category(),
		                        "Failed to detach file descriptor " + std::to_string(fd) + " from epoll");
	}
}

void Reactor::Run()
{
	std::array<epoll_event, 64> buffer = {};

	while (m_running)
	{
		// wait for events
		const int eventCount = epoll_wait(m_epoll.GetFileDescriptor(), buffer.data(), buffer.size(), -1);
		if (eventCount < 0)
		{
			const int code = errno;

			// waiting interrupted by an asynchronous signal is not an error
			if (code != EINTR)
			{
				throw std::system_error(code, std::system_category(), "Failed to wait on epoll");
			}
		}

		// dispatch events
		for (int i = 0; i < eventCount; i++)
		{
			const auto flags = buffer[i].events;

			const bool isRead = (flags & EPOLLIN);
			const bool isWrite = (flags & EPOLLOUT);
			const bool isError = (flags & EPOLLERR) || (flags & EPOLLHUP);

			const int fd = buffer[i].data.fd;

			Dispatch(fd, isRead, isWrite, isError);
		}
	}
}

void Reactor::Stop()
{
	m_running = false;
}

void Reactor::Dispatch(int fd, bool isRead, bool isWrite, bool isError)
{
	if (fd < 0 && static_cast<size_t>(fd) >= m_handlers.size())
	{
		// invalid file descriptor
		return;
	}

	const ReactorHandler& handler = m_handlers[fd];

	if (isError)
	{
		if (handler.onError)
			handler.onError();
		else
			throw std::runtime_error("Unhandled error event on file descriptor " + std::to_string(fd));
	}

	if (isRead)
	{
		if (handler.onRead && IsReadEnabled(handler.events))
			handler.onRead();
		else
			Modify(fd, DisableRead(handler.events));
	}

	if (isWrite)
	{
		if (handler.onWrite && IsWriteEnabled(handler.events))
			handler.onWrite();
		else
			Modify(fd, DisableWrite(handler.events));
	}
}
