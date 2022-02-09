#pragma once

#include <string>
#include <string_view>

#include "HTTPStatus.h"

namespace HTTP
{
	class ResponseBuilder
	{
		std::string m_buffer;

		void AddStatusLine(Status status);

	public:
		explicit ResponseBuilder(Status status);

		ResponseBuilder& AddHeader(const std::string_view& name, const std::string_view& value);
		ResponseBuilder& AddContent(const std::string_view& content, const std::string_view& type);

		std::string Build()
		{
			return std::move(m_buffer);
		}
	};
}
