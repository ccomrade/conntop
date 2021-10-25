#pragma once

#include <functional>
#include <vector>

#include "Handle.h"

enum class ReactorEvents
{
	NONE_EXCEPT_ERRORS, READ_ONLY, WRITE_ONLY, READ_WRITE
};

struct ReactorHandler
{
	ReactorEvents events = ReactorEvents::NONE_EXCEPT_ERRORS;

	std::function<void()> onRead;
	std::function<void()> onWrite;
	std::function<void()> onError;
};

class Reactor
{
	Handle m_epoll;
	bool m_running = false;
	std::vector<ReactorHandler> m_handlers;  // index of each handler is equal to its file descriptor

	void Dispatch(int fd, bool isRead, bool isWrite, bool isError);

public:
	Reactor() = default;

	void Init();

	void Attach(const Handle& handle, ReactorHandler&& handler);
	void Modify(const Handle& handle, ReactorEvents events);
	void Detach(const Handle& handle);

	void Attach(int fd, ReactorHandler&& handler);
	void Modify(int fd, ReactorEvents events);
	void Detach(int fd);

	void Run();

	void Stop();
};
