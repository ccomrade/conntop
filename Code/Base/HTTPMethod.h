#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

namespace HTTP
{
	enum class Method
	{
		GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH
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
			case Method::CONNECT: return "CONNECT";
			case Method::OPTIONS: return "OPTIONS";
			case Method::TRACE:   return "TRACE";
			case Method::PATCH:   return "PATCH";
		}

		return {};
	}

	inline constexpr struct { std::string_view string; Method method; } STRING_METHOD_TABLE[] = {
		{ "GET", Method::GET },
		{ "HEAD", Method::HEAD },
		{ "POST", Method::POST },
		{ "PUT", Method::PUT },
		{ "DELETE", Method::DELETE },
		{ "CONNECT", Method::CONNECT },
		{ "OPTIONS", Method::OPTIONS },
		{ "TRACE", Method::TRACE },
		{ "PATCH", Method::PATCH },
	};

	inline constexpr bool ParseMethod(const std::string_view& string, Method& result)
	{
		for (const auto& entry : STRING_METHOD_TABLE)
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
