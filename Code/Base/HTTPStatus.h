#pragma once

#include <string_view>

namespace HTTP
{
	enum class Status
	{
		CONTINUE                        = 100,
		SWITCHING_PROTOCOLS             = 101,

		OK                              = 200,
		CREATED                         = 201,
		ACCEPTED                        = 202,
		NON_AUTHORITATIVE_INFORMATION   = 203,
		NO_CONTENT                      = 204,
		RESET_CONTENT                   = 205,
		PARTIAL_CONTENT                 = 206,

		MULTIPLE_CHOICES                = 300,
		MOVED_PERMANENTLY               = 301,
		FOUND                           = 302,
		SEE_OTHER                       = 303,
		NOT_MODIFIED                    = 304,
		USE_PROXY                       = 305,
		TEMPORARY_REDIRECT              = 307,
		PERMANENT_REDIRECT              = 308,

		BAD_REQUEST                     = 400,
		UNAUTHORIZED                    = 401,
		PAYMENT_REQUIRED                = 402,
		FORBIDDEN                       = 403,
		NOT_FOUND                       = 404,
		METHOD_NOT_ALLOWED              = 405,
		NOT_ACCEPTABLE                  = 406,
		PROXY_AUTHENTICATION_REQUIRED   = 407,
		REQUEST_TIMEOUT                 = 408,
		CONFLICT                        = 409,
		GONE                            = 410,
		LENGTH_REQUIRED                 = 411,
		PRECONDITION_FAILED             = 412,
		PAYLOAD_TOO_LARGE               = 413,
		URI_TOO_LONG                    = 414,
		UNSUPPORTED_MEDIA_TYPE          = 415,
		RANGE_NOT_SATISFIABLE           = 416,
		EXPECTATION_FAILED              = 417,
		I_AM_A_TEAPOT                   = 418,
		UPGRADE_REQUIRED                = 426,
		PRECONDITION_REQUIRED           = 428,
		TOO_MANY_REQUESTS               = 429,
		REQUEST_HEADER_FIELDS_TOO_LARGE = 431,

		INTERNAL_SERVER_ERROR           = 500,
		NOT_IMPLEMENTED                 = 501,
		BAD_GATEWAY                     = 502,
		SERVICE_UNAVAILABLE             = 503,
		GATEWAY_TIMEOUT                 = 504,
		HTTP_VERSION_NOT_SUPPORTED      = 505,
	};

	inline constexpr std::string_view StatusToString(Status status)
	{
		switch (status)
		{
			case Status::CONTINUE:                        return "Continue";
			case Status::SWITCHING_PROTOCOLS:             return "Switching Protocols";

			case Status::OK:                              return "OK";
			case Status::CREATED:                         return "Created";
			case Status::ACCEPTED:                        return "Accepted";
			case Status::NON_AUTHORITATIVE_INFORMATION:   return "Non-Authoritative Information";
			case Status::NO_CONTENT:                      return "No Content";
			case Status::RESET_CONTENT:                   return "Reset Content";
			case Status::PARTIAL_CONTENT:                 return "Partial Content";

			case Status::MULTIPLE_CHOICES:                return "Multiple Choices";
			case Status::MOVED_PERMANENTLY:               return "Moved Permanently";
			case Status::FOUND:                           return "Found";
			case Status::SEE_OTHER:                       return "See Other";
			case Status::NOT_MODIFIED:                    return "Not Modified";
			case Status::USE_PROXY:                       return "Use Proxy";
			case Status::TEMPORARY_REDIRECT:              return "Temporary Redirect";
			case Status::PERMANENT_REDIRECT:              return "Permanent Redirect";

			case Status::BAD_REQUEST:                     return "Bad Request";
			case Status::UNAUTHORIZED:                    return "Unauthorized";
			case Status::PAYMENT_REQUIRED:                return "Payment Required";
			case Status::FORBIDDEN:                       return "Forbidden";
			case Status::NOT_FOUND:                       return "Not Found";
			case Status::METHOD_NOT_ALLOWED:              return "Method Not Allowed";
			case Status::NOT_ACCEPTABLE:                  return "Not Acceptable";
			case Status::PROXY_AUTHENTICATION_REQUIRED:   return "Proxy Authentication Required";
			case Status::REQUEST_TIMEOUT:                 return "Request Timeout";
			case Status::CONFLICT:                        return "Conflict";
			case Status::GONE:                            return "Gone";
			case Status::LENGTH_REQUIRED:                 return "Length Required";
			case Status::PRECONDITION_FAILED:             return "Precondition Failed";
			case Status::PAYLOAD_TOO_LARGE:               return "Payload Too Large";
			case Status::URI_TOO_LONG:                    return "URI Too Long";
			case Status::UNSUPPORTED_MEDIA_TYPE:          return "Unsupported Media Type";
			case Status::RANGE_NOT_SATISFIABLE:           return "Range Not Satisfiable";
			case Status::EXPECTATION_FAILED:              return "Expectation Failed";
			case Status::I_AM_A_TEAPOT:                   return "I'm a Teapot";
			case Status::UPGRADE_REQUIRED:                return "Upgrade Required";
			case Status::PRECONDITION_REQUIRED:           return "Precondition Required";
			case Status::TOO_MANY_REQUESTS:               return "Too Many Requests";
			case Status::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";

			case Status::INTERNAL_SERVER_ERROR:           return "Internal Server Error";
			case Status::NOT_IMPLEMENTED:                 return "Not Implemented";
			case Status::BAD_GATEWAY:                     return "Bad Gateway";
			case Status::SERVICE_UNAVAILABLE:             return "Service Unavailable";
			case Status::GATEWAY_TIMEOUT:                 return "Gateway Timeout";
			case Status::HTTP_VERSION_NOT_SUPPORTED:      return "HTTP Version Not Supported";
		}

		return {};
	}
}
