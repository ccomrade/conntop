#include "HTTP.h"
#include "HTTPRequestBuilder.h"
#include "Base64.h"

namespace HTTP
{
	std::string RequestBuilder::GetHost(const URL& url)
	{
		std::string host(url.authority.host);

		if (!url.authority.port.empty())
		{
			host += ':';
			host += url.authority.port;
		}

		return host;
	}

	std::string RequestBuilder::GetPath(const URL& url)
	{
		std::string path(url.path.empty() ? "/" : url.path);

		if (!url.query.empty())
		{
			path += '?';
			path += url.query;
		}

		return path;
	}

	void RequestBuilder::AddRequestLine(Method method, const std::string_view& path)
	{
		m_buffer += MethodToString(method);
		m_buffer += ' ';
		m_buffer += path;
		m_buffer += ' ';
		m_buffer += VERSION;
		m_buffer += NEWLINE;
	}

	RequestBuilder::RequestBuilder(Method method, const std::string_view& host, const std::string_view& path)
	{
		AddRequestLine(method, path);
		AddHeader("Host", host);
	}

	RequestBuilder::RequestBuilder(Method method, const std::string_view& url) : RequestBuilder(method, URL::FromString(url))
	{
	}

	RequestBuilder::RequestBuilder(Method method, const URL& url) : RequestBuilder(method, GetHost(url), GetPath(url))
	{
		if (!url.authority.userinfo.empty())
		{
			AddHeader("Authorization", "Basic " + Base64::Encode(url.authority.userinfo));
		}
	}

	RequestBuilder& RequestBuilder::AddHeader(const std::string_view& name, const std::string_view& value)
	{
		m_buffer += name;
		m_buffer += ": ";
		m_buffer += value;
		m_buffer += NEWLINE;

		return *this;
	}

	RequestBuilder& RequestBuilder::AddContent(const std::string_view& content, const std::string_view& type)
	{
		AddHeader("Content-Type", type);
		AddHeader("Content-Length", std::to_string(content.length()));

		m_buffer += NEWLINE;
		m_buffer += content;

		return *this;
	}
}
