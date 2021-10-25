#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <system_error>

#include "System.h"

void System::SetFileDescriptorNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		throw std::system_error(errno, std::system_category(),
		                        "Failed to set file descriptor " + std::to_string(fd) + " non-blocking");
	}
}

void System::SetFileDescriptorCloseOnExec(int fd)
{
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		throw std::system_error(errno, std::system_category(),
		                        "Failed to set file descriptor " + std::to_string(fd) + " close-on-exec");
	}
}
