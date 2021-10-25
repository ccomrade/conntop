#pragma once

#include <string_view>

namespace System
{
	inline constexpr std::string_view NEWLINE = "\n";

	void SetFileDescriptorNonBlocking(int fd);
	void SetFileDescriptorCloseOnExec(int fd);
}
