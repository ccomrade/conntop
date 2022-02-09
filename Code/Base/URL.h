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
		std::string_view::size_type length = 0;

		for (char ch : text)
		{
			if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.')
			{
				length++;
			}
			else if (ch == ':')
			{
				break;
			}
			else
			{
				return false;
			}
		}

		if (length < text.length() && text[length] == ':')
		{
			scheme = text.substr(0, length);

			// remove scheme and ':'
			text.remove_prefix(length + 1);

			return true;
		}
		else
		{
			return false;
		}
	}

	constexpr bool ParseAuthority_Userinfo(std::string_view& text)
	{
		std::string_view::size_type length = 0;

		for (char ch : text)
		{
			if (ch == '/')
			{
				break;
			}
			else if (ch == '@')
			{
				authority.userinfo = text.substr(0, length);

				// remove userinfo and '@'
				text.remove_prefix(length + 1);

				return true;
			}

			length++;
		}

		return false;
	}

	constexpr bool ParseAuthority_Host(std::string_view& text)
	{
		std::string_view::size_type length = 0;

		// IPv6 address
		bool foundOpeningBracket = false;
		bool foundClosingBracket = false;

		for (char ch : text)
		{
			if (ch == '/' || (ch == ':' && (!foundOpeningBracket || foundClosingBracket)))
			{
				break;
			}
			else if (ch == '[')
			{
				if (foundOpeningBracket)
				{
					return false;
				}

				foundOpeningBracket = true;
			}
			else if (ch == ']')
			{
				if (!foundOpeningBracket || foundClosingBracket)
				{
					return false;
				}

				foundClosingBracket = true;
			}

			length++;
		}

		if (length > 0)
		{
			authority.host = text.substr(0, length);

			// remove host and keep ':' or '/'
			text.remove_prefix(length);

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

		std::string_view::size_type length = 0;

		for (char ch : text)
		{
			if (ch == '/')
			{
				break;
			}

			length++;
		}

		if (length > 0)
		{
			authority.port = text.substr(0, length);

			// remove port and keep '/'
			text.remove_prefix(length);

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

		const bool foundUserinfo = ParseAuthority_Userinfo(text);
		const bool foundHost = ParseAuthority_Host(text);
		const bool foundPort = ParseAuthority_Port(text);

		return foundHost || (!foundUserinfo && !foundPort);
	}

	constexpr bool ParsePath(std::string_view& text)
	{
		std::string_view::size_type length = 0;

		for (char ch : text)
		{
			if (ch == '?' || ch == '#')
			{
				break;
			}

			length++;
		}

		path = text.substr(0, length);

		// remove path and keep '?' or '#'
		text.remove_prefix(length);

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

		std::string_view::size_type length = 0;

		for (char ch : text)
		{
			if (ch == '#')
			{
				break;
			}

			length++;
		}

		query = text.substr(0, length);

		// remove path and keep '#'
		text.remove_prefix(length);

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
		text.remove_prefix(text.length());

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
