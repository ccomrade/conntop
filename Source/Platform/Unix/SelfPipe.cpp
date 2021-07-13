/**
 * @file
 * @brief Implementation of SelfPipe class.
 */

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <system_error>

#include "SelfPipe.hpp"

namespace ctp
{
	SelfPipe::SelfPipe()
	: m_pipefd{}
	{
		if (pipe(m_pipefd) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to create self-pipe");
		}

		if (fcntl(getReadFD(), F_SETFL, O_NONBLOCK) < 0 || fcntl(getWriteFD(), F_SETFL, O_NONBLOCK) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to set self-pipe non-blocking");
		}

		if (fcntl(getReadFD(), F_SETFD, FD_CLOEXEC) < 0 || fcntl(getWriteFD(), F_SETFD, FD_CLOEXEC) < 0)
		{
			throw std::system_error(errno, std::system_category(), "Unable to set self-pipe close-on-exec");
		}
	}

	size_t SelfPipe::writeData(const char *data, size_t dataLength)
	{
		ssize_t length = write(getWriteFD(), data, dataLength);
		if (length < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				return 0;
			}
			else
			{
				throw std::system_error(errno, std::system_category(), "Unable to write to self-pipe");
			}
		}

		return length;
	}

	size_t SelfPipe::readData(char *buffer, size_t bufferSize)
	{
		ssize_t length = read(getReadFD(), buffer, bufferSize);
		if (length < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				return 0;
			}
			else
			{
				throw std::system_error(errno, std::system_category(), "Unable to read from self-pipe");
			}
		}

		return length;
	}

	size_t SelfPipe::clear()
	{
		char buffer[2048];
		size_t totalLength = 0;

		while (true)
		{
			size_t length = readData(buffer, sizeof buffer);
			totalLength += length;

			if (length < sizeof buffer)
			{
				break;
			}
		}

		return totalLength;
	}

	void SelfPipe::destroy()
	{
		close(getWriteFD());
		close(getReadFD());
	}
}
