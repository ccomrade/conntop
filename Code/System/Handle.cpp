#include <unistd.h>

#include "Handle.h"

void Handle::Release() noexcept
{
	close(m_fd);

	m_fd = -1;
}
