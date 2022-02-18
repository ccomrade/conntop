#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

struct URL
{
	////////////////////////////////////////////////////////////////////////////////

	std::string_view scheme;

	struct Authority
	{
		std::string_view userinfo;
		std::string_view host;
		std::string_view port;
	};

	Authority authority;

	std::string_view path;
	std::string_view query;
	std::string_view fragment;

	////////////////////////////////////////////////////////////////////////////////

	std::string ToString() const;

	////////////////////////////////////////////////////////////////////////////////

private:
	constexpr bool ParseScheme(std::string_view& text)
	{
		const auto schemeEndPos = text.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789+-.");

		if (schemeEndPos != std::string_view::npos && text[schemeEndPos] == ':')
		{
			scheme = text.substr(0, schemeEndPos);

			// remove scheme and ':'
			text.remove_prefix(scheme.length() + 1);

			return true;
		}
		else
		{
			return false;
		}
	}

	constexpr bool ParseAuthority_Userinfo(std::string_view& text)
	{
		const auto userinfoEndPos = text.find_first_of("@/");

		if (userinfoEndPos != std::string_view::npos && text[userinfoEndPos] == '@')
		{
			authority.userinfo = text.substr(0, userinfoEndPos);

			// remove userinfo and '@'
			text.remove_prefix(authority.userinfo.length() + 1);

			return true;
		}
		else
		{
			return false;
		}
	}

	constexpr bool ParseAuthority_Host(std::string_view& text)
	{
		const auto hostEndPos = text.find('/');

		authority.host = (hostEndPos == std::string_view::npos) ? text : text.substr(0, hostEndPos);

		const auto lastColonPos = authority.host.rfind(':');

		if (lastColonPos != std::string_view::npos)
		{
			// IPv6 addresses contain colons too
			const auto closingBracketPos = authority.host.find(']', lastColonPos + 1);

			if (closingBracketPos == std::string_view::npos)
			{
				// remove port from the host
				authority.host.remove_suffix(authority.host.length() - lastColonPos);
			}
		}

		if (!authority.host.empty())
		{
			// remove host and keep ':' or '/'
			text.remove_prefix(authority.host.length());

			return true;
		}
		else
		{
			return false;
		}
	}

	constexpr bool ParseAuthority_Port(std::string_view& text)
	{
		if (text.empty() || text[0] != ':')
		{
			return false;
		}

		// remove ':'
		text.remove_prefix(1);

		const auto portEndPos = text.find('/');

		authority.port = (portEndPos == std::string_view::npos) ? text : text.substr(0, portEndPos);

		if (!authority.port.empty())
		{
			// remove port and keep '/'
			text.remove_prefix(authority.port.length());

			return true;
		}
		else
		{
			return false;
		}
	}

	constexpr bool ParseAuthority(std::string_view& text)
	{
		if (text.length() < 2 || text.substr(0, 2) != "//")
		{
			// authority is optional
			return true;
		}

		// remove "//"
		text.remove_prefix(2);

		const bool hasUserinfo = ParseAuthority_Userinfo(text);
		const bool hasHost = ParseAuthority_Host(text);
		const bool hasPort = ParseAuthority_Port(text);

		return hasHost || (!hasUserinfo && !hasPort);
	}

	constexpr bool ParsePath(std::string_view& text)
	{
		if (!text.empty() && text[0] != '/')
		{
			// path must begin with '/'
			return false;
		}

		const auto pathEndPos = text.find_first_of("?#");

		path = (pathEndPos == std::string_view::npos) ? text : text.substr(0, pathEndPos);

		// remove path and keep '?' or '#'
		text.remove_prefix(path.length());

		return true;
	}

	constexpr bool ParseQuery(std::string_view& text)
	{
		if (text.empty() || text[0] != '?')
		{
			// query is optional
			return true;
		}

		// remove '?'
		text.remove_prefix(1);

		const auto queryEndPos = text.find('#');

		query = (queryEndPos == std::string_view::npos) ? text : text.substr(0, queryEndPos);

		// remove query and keep '#'
		text.remove_prefix(query.length());

		return true;
	}

	constexpr bool ParseFragment(std::string_view& text)
	{
		if (text.empty() || text[0] != '#')
		{
			// fragment is optional
			return true;
		}

		// remove '#'
		text.remove_prefix(1);

		fragment = text;

		// remove fragment
		text.remove_prefix(fragment.length());

		return true;
	}

public:
	constexpr bool Parse(std::string_view text)
	{
		*this = {};

		return ParseScheme(text)
		    && ParseAuthority(text)
		    && ParsePath(text)
		    && ParseQuery(text)
		    && ParseFragment(text)
		    && text.empty();
	}

	static constexpr URL FromString(const std::string_view& text)
	{
		URL url;
		if (!url.Parse(text))
		{
			throw std::invalid_argument("Invalid URL '" + std::string(text) + "'");
		}

		return url;
	}

	////////////////////////////////////////////////////////////////////////////////
};
