#include <unistd.h>
#include <errno.h>
#include <system_error>

#include "Log.h"
#include "System.h"

namespace
{
	std::string_view GetSeverityPrefix(LogSeverity severity)
	{
		switch (severity)
		{
			case LogSeverity::ERROR:
				return "<3>";
			case LogSeverity::WARNING:
				return "<4>";
			case LogSeverity::NOTICE:
				return "<5>";
			case LogSeverity::INFO:
				return "<6>";
			case LogSeverity::DEBUG:
				return "<7>";
		}

		return {};
	}
}

void Log::Write(LogSeverity severity, const std::string_view& message)
{
	if (!IsSeverityEnabled(severity))
	{
		// drop messages above the current verbosity level
		return;
	}

	// let systemd-journal know about message severity
	const std::string_view messagePrefix = GetSeverityPrefix(severity);

	std::string buffer;
	buffer.reserve(messagePrefix.length() + message.length() + System::NEWLINE.length());

	buffer += messagePrefix;
	buffer += message;
	buffer += System::NEWLINE;

	// use write syscall directly to ensure thread safety and no caching
	if (write(STDERR_FILENO, buffer.c_str(), buffer.length()) < 0)
	{
		// disable the log to avoid recursive throw in case of write error
		config.verbosity = -1;

		throw std::system_error(errno, std::system_category(), "Log write failed");
	}
}
