#pragma once

#include <string>
#include <string_view>
#include <stdexcept>

#include "URLParser.h"

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

	constexpr bool Parse(const std::string_view& text)
	{
		std::string_view remainingText = text;

		const auto [isSchemeOK, newScheme] = URLParser::ParseScheme(remainingText);
		if (!isSchemeOK)
			return false;

		const auto [isAuthorityOK, newUserinfo, newHost, newPort] = URLParser::ParseAuthority(remainingText);
		if (!isAuthorityOK)
			return false;

		const auto [isPathOK, newPath] = URLParser::ParsePath(remainingText);
		if (!isPathOK)
			return false;

		const auto [isQueryOK, newQuery] = URLParser::ParseQuery(remainingText);
		if (!isQueryOK)
			return false;

		const auto [isFragmentOK, newFragment] = URLParser::ParseFragment(remainingText);
		if (!isFragmentOK)
			return false;

		if (!remainingText.empty())
			return false;

		this->scheme = newScheme;
		this->authority.userinfo = newUserinfo;
		this->authority.host = newHost;
		this->authority.port = newPort;
		this->path = newPath;
		this->query = newQuery;
		this->fragment = newFragment;

		return true;
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
