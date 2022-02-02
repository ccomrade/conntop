#pragma once

#include <atomic>
#include <string>
#include <string_view>

#include "Base/Format.h"

class Log
{
public:
	enum class Severity
	{
		ERROR, WARNING, NOTICE, INFO, DEBUG
	};

private:
	std::atomic<int> m_verbosity = 0;

	Log() = default;

public:
	static Log& GetInstance()
	{
		static Log instance;
		return instance;
	}

	int GetVerbosity() const
	{
		return m_verbosity.load(std::memory_order_relaxed);
	}

	void SetVerbosity(int verbosity)
	{
		m_verbosity.store(verbosity, std::memory_order_relaxed);
	}

	bool IsSeverityEnabled(Severity severity) const
	{
		const int verbosity = GetVerbosity();

		switch (severity)
		{
			case Severity::ERROR:
			case Severity::WARNING:
			case Severity::NOTICE:
				return verbosity >= 0;
			case Severity::INFO:
				return verbosity >= 1;
			case Severity::DEBUG:
				return verbosity >= 2;
		}

		return false;
	}

	void Write(Severity severity, const std::string_view& message);
};

////////////////////////////////////////////////////////////////////////////////

#define LOG(severity, message) if (Log::GetInstance().IsSeverityEnabled(severity)) Log::GetInstance().Write(severity, message)

#define LOG_ERROR(message) LOG(Log::Severity::ERROR, message)
#define LOG_WARNING(message) LOG(Log::Severity::WARNING, message)
#define LOG_NOTICE(message) LOG(Log::Severity::NOTICE, message)
#define LOG_INFO(message) LOG(Log::Severity::INFO, message)
#define LOG_DEBUG(message) LOG(Log::Severity::DEBUG, message)

////////////////////////////////////////////////////////////////////////////////
