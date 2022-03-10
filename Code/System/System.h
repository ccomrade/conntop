#pragma once

#include <cerrno>
#include <string>
#include <string_view>
#include <system_error>

#include <fmt/format.h>

namespace System
{
	inline constexpr std::string_view NEWLINE = "\n";

	void CloseFileDescriptor(int fd);

	void SetFileDescriptorNonBlocking(int fd);
	void SetFileDescriptorCloseOnExec(int fd);

	template<class... Args>
	std::system_error Error(fmt::format_string<Args...> format, Args&&... args)
	{
		return { errno, std::system_category(), fmt::format(format, std::forward<Args>(args)...) };
	}

	template<class... Args>
	std::system_error Error(int code, fmt::format_string<Args...> format, Args&&... args)
	{
		return { code, std::system_category(), fmt::format(format, std::forward<Args>(args)...) };
	}
}
