#include "HTTP.h"
#include "HTTPResponseBuilder.h"

namespace HTTP
{
	void ResponseBuilder::AddStatusLine(Status status)
	{
		m_buffer += VERSION;
		m_buffer += ' ';
		m_buffer += std::to_string(static_cast<unsigned int>(status));
		m_buffer += ' ';
		m_buffer += StatusToString(status);
		m_buffer += NEWLINE;
	}

	ResponseBuilder::ResponseBuilder(Status status)
	{
		AddStatusLine(status);
	}

	ResponseBuilder& ResponseBuilder::AddHeader(const std::string_view& name, const std::string_view& value)
	{
		m_buffer += name;
		m_buffer += ": ";
		m_buffer += value;
		m_buffer += NEWLINE;

		return *this;
	}

	ResponseBuilder& ResponseBuilder::AddContent(const std::string_view& content, const std::string_view& type)
	{
		AddHeader("Content-Type", type);
		AddHeader("Content-Length", std::to_string(content.length()));

		m_buffer += NEWLINE;
		m_buffer += content;

		return *this;
	}
}
