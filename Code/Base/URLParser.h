#pragma once

#include <string_view>
#include <tuple>

namespace URLParser
{
	inline constexpr std::tuple<bool, std::string_view> ParseScheme(std::string_view& text)
	{
		const auto schemeEndPos = text.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789+-.");

		if (schemeEndPos != std::string_view::npos && text[schemeEndPos] == ':')
		{
			const std::string_view scheme = text.substr(0, schemeEndPos);

			// remove scheme and ':'
			text.remove_prefix(schemeEndPos + 1);

			return { true, scheme };
		}
		else
		{
			return {};
		}
	}

	inline constexpr std::tuple<bool, std::string_view, std::string_view, std::string_view> ParseAuthority(std::string_view& text)
	{
		constexpr auto userinfoParser = [](std::string_view& text) -> std::tuple<bool, std::string_view>
		{
			const auto userinfoEndPos = text.find_first_of("@/");

			if (userinfoEndPos != std::string_view::npos && text[userinfoEndPos] == '@')
			{
				const std::string_view userinfo = text.substr(0, userinfoEndPos);

				// remove userinfo and '@'
				text.remove_prefix(userinfoEndPos + 1);

				return { true, userinfo };
			}
			else
			{
				return {};
			}
		};

		constexpr auto hostParser = [](std::string_view& text) -> std::tuple<bool, std::string_view>
		{
			const std::string_view host = [&text]()
			{
				const auto hostLength = [&text]()
				{
					const auto hostEndPos = text.find('/');

					return (hostEndPos == std::string_view::npos) ? text.length() : hostEndPos;
				}();

				auto host = text.substr(0, hostLength);

				if (const auto lastColonPos = host.rfind(':'); lastColonPos != std::string_view::npos)
				{
					// IPv6 addresses contain colons too
					const auto closingBracketPos = host.find(']', lastColonPos + 1);

					if (closingBracketPos == std::string_view::npos)
					{
						// remove port from the host
						host = text.substr(0, lastColonPos);
					}
				}

				return host;
			}();

			if (!host.empty())
			{
				// remove host and keep ':' or '/'
				text.remove_prefix(host.length());

				return { true, host };
			}
			else
			{
				return {};
			}
		};

		constexpr auto portParser = [](std::string_view& text) -> std::tuple<bool, std::string_view>
		{
			if (text.empty() || text[0] != ':')
			{
				return {};
			}

			// remove ':'
			text.remove_prefix(1);

			const auto portLength = [&text]()
			{
				const auto portEndPos = text.find('/');

				return (portEndPos == std::string_view::npos) ? text.length() : portEndPos;
			}();

			if (portLength > 0)
			{
				const std::string_view port = text.substr(0, portLength);

				// remove port and keep '/'
				text.remove_prefix(portLength);

				return { true, port };
			}
			else
			{
				return {};
			}
		};

		if (text.length() < 2 || text.substr(0, 2) != "//")
		{
			// authority is optional
			return { true, {}, {}, {} };
		}

		// remove "//"
		text.remove_prefix(2);

		const auto [hasUserinfo, userinfo] = userinfoParser(text);
		const auto [hasHost, host] = hostParser(text);
		const auto [hasPort, port] = portParser(text);

		if (hasHost || (!hasUserinfo && !hasPort))
		{
			return { true, userinfo, host, port };
		}
		else
		{
			return {};
		}
	}

	inline constexpr std::tuple<bool, std::string_view> ParsePath(std::string_view& text)
	{
		const auto pathLength = [&text]()
		{
			const auto pathEndPos = text.find_first_of("?#");

			return (pathEndPos == std::string_view::npos) ? text.length() : pathEndPos;
		}();

		const std::string_view path = text.substr(0, pathLength);

		// remove path and keep '?' or '#'
		text.remove_prefix(pathLength);

		return { true, path };
	}

	inline constexpr std::tuple<bool, std::string_view> ParseQuery(std::string_view& text)
	{
		if (text.empty() || text[0] != '?')
		{
			// query is optional
			return { true, {} };
		}

		// remove '?'
		text.remove_prefix(1);

		const auto queryLength = [&text]()
		{
			const auto queryEndPos = text.find('#');

			return (queryEndPos == std::string_view::npos) ? text.length() : queryEndPos;
		}();

		const std::string_view query = text.substr(0, queryLength);

		// remove query and keep '#'
		text.remove_prefix(queryLength);

		return { true, query };
	}

	inline constexpr std::tuple<bool, std::string_view> ParseFragment(std::string_view& text)
	{
		if (text.empty() || text[0] != '#')
		{
			// fragment is optional
			return { true, {} };
		}

		// remove '#'
		text.remove_prefix(1);

		const std::string_view fragment = text;

		// remove fragment
		text.remove_prefix(fragment.length());

		return { true, fragment };
	}
}
