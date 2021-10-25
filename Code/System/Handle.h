#pragma once

#include <utility>

class Handle
{
	// file descriptor
	int m_fd = -1;

	void Release() noexcept;

public:
	Handle() = default;

	explicit Handle(int fd) : m_fd(fd)
	{
	}

	Handle(const Handle&) = delete;

	Handle(Handle&& other) noexcept
	{
		Swap(other);
	}

	Handle& operator=(const Handle&) = delete;

	Handle& operator=(Handle&& other) noexcept
	{
		Close();
		Swap(other);

		return *this;
	}

	~Handle()
	{
		Close();
	}

	bool IsOpen() const
	{
		return m_fd >= 0;
	}

	int GetFileDescriptor() const
	{
		return m_fd;
	}

	void Close()
	{
		if (IsOpen())
		{
			Release();
		}
	}

	void Swap(Handle& other)
	{
		std::swap(m_fd, other.m_fd);
	}

	explicit operator bool() const
	{
		return IsOpen();
	}
};
