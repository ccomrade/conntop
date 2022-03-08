#include <unistd.h>

#include "Log.h"
#include "System.h"

void Log::WriteMessageAlways(Severity severity, const std::string_view& message)
{
	// let systemd-journal know about message severity
	const std::string_view messagePrefix = [severity]() -> std::string_view
	{
		switch (severity)
		{
			case Log::Severity::ERROR:   return "<3>";
			case Log::Severity::WARNING: return "<4>";
			case Log::Severity::NOTICE:  return "<5>";
			case Log::Severity::INFO:    return "<6>";
			case Log::Severity::DEBUG:   return "<7>";
		}

		return {};
	}();

	const std::size_t messageLength = messagePrefix.length() + message.length() + System::NEWLINE.length();

	std::string buffer;
	buffer.reserve(messageLength);

	buffer += messagePrefix;
	buffer += message;
	buffer += System::NEWLINE;

	// use write syscall directly to ensure thread safety and no caching
	if (write(STDERR_FILENO, buffer.c_str(), buffer.length()) < 0)
	{
		// disable the log to avoid recursive throw in case of write error
		SetVerbosity(Verbosity::DISABLED);

		throw System::Error("Log write failed");
	}
}
