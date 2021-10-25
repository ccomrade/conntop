#pragma once

#include <atomic>
#include <string>
#include <string_view>

#include "Base/Format.h"

enum class LogSeverity
{
	ERROR, WARNING, NOTICE, INFO, DEBUG
};

struct LogConfig
{
	std::atomic<int> verbosity = 0;
};

namespace Log
{
	////////////////////////
	inline LogConfig config;
	////////////////////////

	inline bool IsSeverityEnabled(LogSeverity severity)
	{
		const int verbosity = config.verbosity;

		switch (severity)
		{
			case LogSeverity::ERROR:
			case LogSeverity::WARNING:
			case LogSeverity::NOTICE:
				return verbosity >= 0;
			case LogSeverity::INFO:
				return verbosity >= 1;
			case LogSeverity::DEBUG:
				return verbosity >= 2;
		}

		return false;
	}

	void Write(LogSeverity severity, const std::string_view& message);
}

// avoid construction of dropped log messages
#define LOG_BASE(severity, message) if (Log::IsSeverityEnabled(severity)) Log::Write(severity, message)

#define LOG_ERROR(message) LOG_BASE(LogSeverity::ERROR, message)
#define LOG_WARNING(message) LOG_BASE(LogSeverity::WARNING, message)
#define LOG_NOTICE(message) LOG_BASE(LogSeverity::NOTICE, message)
#define LOG_INFO(message) LOG_BASE(LogSeverity::INFO, message)
#define LOG_DEBUG(message) LOG_BASE(LogSeverity::DEBUG, message)
