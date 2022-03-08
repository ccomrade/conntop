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

	template<class Format, class... Args>
	void WriteFormatAlways(Severity severity, Format&& format, Args&&... args)
	{
		// build the message only when it's really needed
		const std::string message = fmt::format(std::forward<Format>(format), std::forward<Args>(args)...);

		WriteMessageAlways(severity, message);
	}

	template<class Format, class... Args>
	void WriteFormat(Severity severity, Format&& format, Args&&... args)
	{
		if (IsSeverityEnabled(severity))
		{
			WriteFormatAlways(severity, std::forward<Format>(format), std::forward<Args>(args)...);
		}
	}

	////////////////////////////////////////////////////////////////////////////////

	template<class Format, class... Args>
	static void Error(Format&& format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::ERROR, std::forward<Format>(format), std::forward<Args>(args)...);
	}

	template<class Format, class... Args>
	static void Warning(Format&& format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::WARNING, std::forward<Format>(format), std::forward<Args>(args)...);
	}

	template<class Format, class... Args>
	static void Notice(Format&& format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::NOTICE, std::forward<Format>(format), std::forward<Args>(args)...);
	}

	template<class Format, class... Args>
	static void Info(Format&& format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::INFO, std::forward<Format>(format), std::forward<Args>(args)...);
	}

	template<class Format, class... Args>
	static void Debug(Format&& format, Args&&... args)
	{
		GetInstance().WriteFormat(Severity::DEBUG, std::forward<Format>(format), std::forward<Args>(args)...);
	}

	////////////////////////////////////////////////////////////////////////////////
};
