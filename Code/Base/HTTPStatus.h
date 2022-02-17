#pragma once

#include <string_view>

namespace HTTP
{
	enum class Status
	{
		SWITCHING_PROTOCOLS   = 101,

		OK                    = 200,
		CREATED               = 201,
		ACCEPTED              = 202,
		NO_CONTENT            = 204,

		MOVED_PERMANENTLY     = 301,
		FOUND                 = 302,
		SEE_OTHER             = 303,
		NOT_MODIFIED          = 304,
		TEMPORARY_REDIRECT    = 307,

		BAD_REQUEST           = 400,
		UNAUTHORIZED          = 401,
		FORBIDDEN             = 403,
		NOT_FOUND             = 404,
		METHOD_NOT_ALLOWED    = 405,

		INTERNAL_SERVER_ERROR = 500,
		NOT_IMPLEMENTED       = 501,
		BAD_GATEWAY           = 502,
		SERVICE_UNAVAILABLE   = 503,
		GATEWAY_TIMEOUT       = 504,
	};

	inline constexpr std::string_view StatusToString(Status status)
	{
		switch (status)
		{
			case Status::SWITCHING_PROTOCOLS:   return "Switching Protocols";
			case Status::OK:                    return "OK";
			case Status::CREATED:               return "Created";
			case Status::ACCEPTED:              return "Accepted";
			case Status::NO_CONTENT:            return "No Content";
			case Status::MOVED_PERMANENTLY:     return "Moved Permanently";
			case Status::FOUND:                 return "Found";
			case Status::SEE_OTHER:             return "See Other";
			case Status::NOT_MODIFIED:          return "Not Modified";
			case Status::TEMPORARY_REDIRECT:    return "Temporary Redirect";
			case Status::BAD_REQUEST:           return "Bad Request";
			case Status::UNAUTHORIZED:          return "Unauthorized";
			case Status::FORBIDDEN:             return "Forbidden";
			case Status::NOT_FOUND:             return "Not Found";
			case Status::METHOD_NOT_ALLOWED:    return "Method Not Allowed";
			case Status::INTERNAL_SERVER_ERROR: return "Internal Server Error";
			case Status::NOT_IMPLEMENTED:       return "Not Implemented";
			case Status::BAD_GATEWAY:           return "Bad Gateway";
			case Status::SERVICE_UNAVAILABLE:   return "Service Unavailable";
			case Status::GATEWAY_TIMEOUT:       return "Gateway Timeout";
		}

		return {};
	}
}
