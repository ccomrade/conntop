#include <unistd.h>
#include <errno.h>
#include <system_error>

#include "Handle.h"
#include "Log.h"

void Handle::Release() noexcept
{
	if (close(m_fd) < 0)
	{
		LOG_ERROR(std::system_error(errno, std::system_category(),
					    "Failed to close file descriptor " + std::to_string(m_fd)).what());
	}

	m_fd = -1;
}
