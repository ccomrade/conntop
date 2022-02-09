#pragma once

#include <string>
#include <string_view>

#include "HTTPMethod.h"
#include "URL.h"

namespace HTTP
{
	class RequestBuilder
	{
		std::string m_buffer;

		static std::string GetHost(const URL& url);
		static std::string GetPath(const URL& url);

		void AddRequestLine(Method method, const std::string_view& path);

	public:
		explicit RequestBuilder(Method method, const std::string_view& host, const std::string_view& path);
		explicit RequestBuilder(Method method, const std::string_view& url);
		explicit RequestBuilder(Method method, const URL& url);

		RequestBuilder& AddHeader(const std::string_view& name, const std::string_view& value);
		RequestBuilder& AddContent(const std::string_view& content, const std::string_view& type);

		std::string Build()
		{
			return std::move(m_buffer);
		}
	};
}
