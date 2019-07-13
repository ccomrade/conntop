/**
 * @file
 * @brief Command line options.
 */

#include "CmdLine.hpp"
#include "conntop_config.h"

namespace ctp
{
	const std::map<KString, CmdLineArgConfig> CmdLine::OPTIONS = {
		{
			"help",
			{
				"h",
				"Show this help and exit."
			}
		},
		{
			"version",
			{
				"",
				"Show version information and exit."
			}
		},
		{
			"verbose",
			{
				"v",
				"Increase log verbosity - 1x normal, 2x high, 3x debug."
			}
		},
		{
			"log-file",
			{
				"L",
				"Use FILE for logging instead of standard error output.",
				ECmdLineArgValue::REQUIRED,
				"FILE"
			}
		},
		{
			"log-style",
			{
				"",
				"Set log format (simple, printk, timestamp, datetime).",
				ECmdLineArgValue::REQUIRED,
				"STYLE"
			}
		},
		{
			"log-color",
			{
				"",
				"Colorize log messages (never, auto, always).",
				ECmdLineArgValue::OPTIONAL,
				"WHEN"
			}
		},
		{
			"log-clear",
			{
				"",
				"Make log file empty on each start."
			}
		},
		{
			"log-utc",
			{
				"",
				"Use UTC time instead of local time in log."
			}
		},
	#ifndef CONNTOP_DEDICATED
		{
			"no-hostname",
			{
				"n",
				"Disable address hostname resolving."
			}
		},
		{
			"no-servname",
			{
				"P",
				"Disable port service name resolving."
			}
		},
		{
			"no-geoip",
			{
				"",
				"Disable GeoIP database."
			}
		},
		{
			"connect",
			{
				"c",
				"Start conntop client and try to connect to SERVER.",
				ECmdLineArgValue::REQUIRED,
				"SERVER"
			}
		},
		{
			"server",
			{
				"s",
				"Start conntop server."
			}
		},
	#endif
		{
			"name",
			{
				"N",
				"Use NAME as client/server name instead of hostname.",
				ECmdLineArgValue::REQUIRED,
				"NAME"
			}
		},
		{
			"port",
			{
				"p",
				"Use PORT instead of default port.",
				ECmdLineArgValue::REQUIRED,
				"PORT"
			}
		},
		{
			"listen",
			{
				"l",
				"Bind server port to ADDRESS.",
				ECmdLineArgValue::REQUIRED,
				"ADDRESS"
			}
		},
		{
			"listen-any",
			{
				"",
				"Bind server port to 0.0.0.0 and [::]."
			}
		}
	};
}
