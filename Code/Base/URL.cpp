#include "URL.h"

std::string URL::ToString() const
{
	std::string url;

	url += scheme;
	url += ":";

	if (!authority.host.empty())
	{
		url += "//";

		if (!authority.userinfo.empty())
		{
			url += authority.userinfo;
			url += "@";
		}

		url += authority.host;

		if (!authority.port.empty())
		{
			url += ":";
			url += authority.port;
		}
	}

	url += path;

	if (!query.empty())
	{
		url += "?";
		url += query;
	}

	if (!fragment.empty())
	{
		url += "#";
		url += fragment;
	}

	return url;
}
