#pragma once

#include <atomic>
#include <string>
#include <string_view>

#include <fmt/format.h>

class Log
{
public:
	enum class Severity
	{
		ERROR, WARNING, NOTICE, INFO, DEBUG
	};

	enum class Verbosity
	{
		DISABLED = -1, LOW = 0, HIGH, DEBUG
	};

private:
	std::atomic<Verbosity> m_verbosity = Verbosity::LOW;

	Log() = default;

public:
	////////////////////////////////////////////////////////////////////////////////

	static Log& GetInstance()
	{
		static Log instance;
		return instance;
	}

	Verbosity GetVerbosity() const
	{
		return m_verbosity.load(std::memory_order_relaxed);
	}

	void SetVerbosity(Verbosity verbosity)
	{
		m_verbosity.store(verbosity, std::memory_order_relaxed);
	}

	bool IsSeverityEnabled(Severity severity) const
	{
		const Verbosity verbosity = GetVerbosity();

		switch (severity)
		{
			case Severity::ERROR:
			case Severity::WARNING:
			case Severity::NOTICE:
				return verbosity >= Verbosity::LOW;
			case Severity::INFO:
				return verbosity >= Verbosity::HIGH;
			case Severity::DEBUG:
				return verbosity >= Verbosity::DEBUG;
		}

		return false;
	}

	////////////////////////////////////////////////////////////////////////////////

	void WriteMessageAlways(Severity severity, const std::string_view& message);

	void WriteMessage(Severity severity, const std::string_view& message)
	{
		if (IsSeverityEnabled(severity))
		{
			WriteMessageAlways(severity, message);
		}
	}

	template<class... Args>
	void WriteFormatAlways(Severity severity, fmt::format_string<Args...> format, Args&&... args)
	{
		WriteMessageAlways(severity, fmt::format(format, std::forward<Args>(args)...));
	}

	template<class... Args>
	void WriteFormat(Severity severity, fmt::format_string<Args...> format, Args&&... args)
	{
		if (IsSeverityEnabled(severity))
		{
			WriteFormatAlways(severity, format, std::forward<Args>(args)...);
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	template<class... Args>
	static void Error(fmt::format_string<Args...> format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::ERROR, format, std::forward<Args>(args)...);
	}

	template<class... Args>
	static void Warning(fmt::format_string<Args...> format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::WARNING, format, std::forward<Args>(args)...);
	}

	template<class... Args>
	static void Notice(fmt::format_string<Args...> format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::NOTICE, format, std::forward<Args>(args)...);
	}

	template<class... Args>
	static void Info(fmt::format_string<Args...> format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::INFO, format, std::forward<Args>(args)...);
	}

	template<class... Args>
	static void Debug(fmt::format_string<Args...> format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::DEBUG, format, std::forward<Args>(args)...);
	}

	////////////////////////////////////////////////////////////////////////////////
};
