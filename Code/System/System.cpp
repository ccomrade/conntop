#include <unistd.h>
#include <fcntl.h>

#include "System.h"

void System::CloseFileDescriptor(int fd)
{
	close(fd);
}

void System::SetFileDescriptorNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		throw System::Error("Failed to set file descriptor {} non-blocking", fd);
	}
}

void System::SetFileDescriptorCloseOnExec(int fd)
{
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
	{
		throw System::Error("Failed to set file descriptor {} close-on-exec", fd);
	}
}
