#pragma once

#include <string_view>

namespace HTTP
{
	inline constexpr std::string_view NEWLINE = "\r\n";

	inline constexpr std::string_view VERSION = "HTTP/1.1";
}
