#pragma once

#include <string_view>

namespace HTTP
{
	enum class Status
	{
		SwitchingProtocols  = 101,

		OK                  = 200,
		Created             = 201,
		Accepted            = 202,
		NoContent           = 204,

		MovedPermanently    = 301,
		Found               = 302,
		SeeOther            = 303,
		NotModified         = 304,
		TemporaryRedirect   = 307,

		BadRequest          = 400,
		Unauthorized        = 401,
		Forbidden           = 403,
		NotFound            = 404,
		MethodNotAllowed    = 405,

		InternalServerError = 500,
		NotImplemented      = 501,
		BadGateway          = 502,
		ServiceUnavailable  = 503,
		GatewayTimeout      = 504,
	};

	constexpr std::string_view StatusToString(Status status)
	{
		switch (status)
		{
			case Status::SwitchingProtocols:  return "Switching Protocols";
			case Status::OK:                  return "OK";
			case Status::Created:             return "Created";
			case Status::Accepted:            return "Accepted";
			case Status::NoContent:           return "No Content";
			case Status::MovedPermanently:    return "Moved Permanently";
			case Status::Found:               return "Found";
			case Status::SeeOther:            return "See Other";
			case Status::NotModified:         return "Not Modified";
			case Status::TemporaryRedirect:   return "Temporary Redirect";
			case Status::BadRequest:          return "Bad Request";
			case Status::Unauthorized:        return "Unauthorized";
			case Status::Forbidden:           return "Forbidden";
			case Status::NotFound:            return "Not Found";
			case Status::MethodNotAllowed:    return "Method Not Allowed";
			case Status::InternalServerError: return "Internal Server Error";
			case Status::NotImplemented:      return "Not Implemented";
			case Status::BadGateway:          return "Bad Gateway";
			case Status::ServiceUnavailable:  return "Service Unavailable";
			case Status::GatewayTimeout:      return "Gateway Timeout";
		}

		return {};
	}
}
