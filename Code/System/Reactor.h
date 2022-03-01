#pragma once

#include <functional>
#include <vector>

#include "Handle.h"

class Reactor
{
public:
	enum class CallbackResult
	{
		STOP, CONTINUE
	};

	using Callback = std::function<CallbackResult()>;

private:
	struct Handler
	{
		// true if the corresponding file descriptor is registered in the epoll instance inside the kernel
		bool isRegistered = false;

		// the current state of the epoll instance inside the kernel
		bool isReadEnabled = false;
		bool isWriteEnabled = false;
		bool isErrorEnabled = false;

		// disabled callbacks are empty
		Callback onRead;
		Callback onWrite;
		Callback onError;
	};

	Handle m_epoll;
	bool m_isRunning = false;
	bool m_isCommitNeeded = false;
	std::vector<Handler> m_handlers;  // index of each handler is its file descriptor

	Handler& GetHandler(int fd);

	void CommitAllChanges();
	void Commit(int fd, Handler& handler);

	void DispatchEvent(int fd, bool isRead, bool isWrite, bool isError);
	void ExecuteCallback(Callback& callback);

public:
	Reactor() = default;

	void Init();

	void EventLoop();
	void StopEventLoop();

	void AttachReadHandler(const Handle& handle, Callback&& callback);
	void AttachWriteHandler(const Handle& handle, Callback&& callback);
	void AttachErrorHandler(const Handle& handle, Callback&& callback);
	void Detach(const Handle& handle);

	void AttachReadHandler(int fd, Callback&& callback);
	void AttachWriteHandler(int fd, Callback&& callback);
	void AttachErrorHandler(int fd, Callback&& callback);
	void Detach(int fd);
};
