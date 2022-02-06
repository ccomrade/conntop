#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

namespace Base64
{
	std::string Encode(const std::string_view& text);
	std::string Decode(const std::string_view& text);
}
