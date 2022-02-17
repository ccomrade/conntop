#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

namespace HTTP
{
	enum class Method
	{
		GET, HEAD, POST, PUT, DELETE, OPTIONS
	};

	inline constexpr std::string_view MethodToString(Method method)
	{
		switch (method)
		{
			case Method::GET:     return "GET";
			case Method::HEAD:    return "HEAD";
			case Method::POST:    return "POST";
			case Method::PUT:     return "PUT";
			case Method::DELETE:  return "DELETE";
			case Method::OPTIONS: return "OPTIONS";
		}

		return {};
	}

	inline constexpr bool ParseMethod(const std::string_view& string, Method& result)
	{
		constexpr struct { std::string_view string; Method method; } TABLE[] = {
			{ "GET", Method::GET },
			{ "HEAD", Method::HEAD },
			{ "POST", Method::POST },
			{ "PUT", Method::PUT },
			{ "DELETE", Method::DELETE },
			{ "OPTIONS", Method::OPTIONS },
		};

		for (const auto& entry : TABLE)
		{
			if (entry.string == string)
			{
				result = entry.method;
				return true;
			}
		}

		return false;
	}

	inline constexpr Method MethodFromString(const std::string_view& string)
	{
		Method method = {};
		if (!ParseMethod(string, method))
		{
			throw std::invalid_argument("Unknown HTTP method '" + std::string(string) + "'");
		}

		return method;
	}
}
