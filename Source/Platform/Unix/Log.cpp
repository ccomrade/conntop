/**
 * @file
 * @brief Implementation of Log class for Unix platform.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>  // std::memcpy
#include <cstdio>  // std::vsnprintf
#include <cerrno>
#include <system_error>
#include <memory>

#include "Log.hpp"
#include "CharBuffer.hpp"
#include "Platform.hpp"
#include "Exception.hpp"
#include "Util.hpp"

static constexpr KString PREFIX_TABLE_SIMPLE[] = {
	[Log::ALWAYS]  = "",
	[Log::ERROR]   = "[ERROR] ",
	[Log::WARNING] = "[WARNING] ",
	[Log::NOTICE]  = "[NOTICE] ",
	[Log::INFO]    = "[INFO] ",
	[Log::DEBUG]   = "[DBG] "
};

static constexpr KString PREFIX_TABLE_PRINTK[] = {
	[Log::ALWAYS]  = "<6>",  // same as INFO
	[Log::ERROR]   = "<3>",
	[Log::WARNING] = "<4>",
	[Log::NOTICE]  = "<5>",
	[Log::INFO]    = "<6>",
	[Log::DEBUG]   = "<7>"
};

static constexpr KString COLOR_PREFIX_TABLE[] = {
	[Log::ALWAYS]  = "",
	[Log::ERROR]   = "\e[1;31m",  // bright red
	[Log::WARNING] = "\e[1;33m",  // bright yellow
	[Log::NOTICE]  = "\e[1;35m",  // bright magenta
	[Log::INFO]    = "",
	[Log::DEBUG]   = "\e[32m"     // green
};

static constexpr KString COLOR_SUFFIX_TABLE[] = {
	[Log::ALWAYS]  = "",
	[Log::ERROR]   = "\e[0m",
	[Log::WARNING] = "\e[0m",
	[Log::NOTICE]  = "\e[0m",
	[Log::INFO]    = "",
	[Log::DEBUG]   = "\e[0m"
};

Log::Log(EVerbosity verbosity, EColorize colorize, EStyle style)
: m_verbosity(verbosity),
  m_colorize(colorize),
  m_style(style),
  m_fileName(),
  m_isColorizeEnabled(),
  m_fd()
{
	m_fd = STDERR_FILENO;

	initColorize();
}

Log::Log(EVerbosity verbosity, EColorize colorize, EStyle style, const KString & fileName, bool clear)
: m_verbosity(verbosity),
  m_colorize(colorize),
  m_style(style),
  m_fileName(fileName),
  m_isColorizeEnabled(),
  m_fd()
{
	int flags = O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC;
	if (clear)
	{
		flags |= O_TRUNC;
	}

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	m_fd = open(fileName.c_str(), flags, mode);
	if (m_fd < 0)
	{
		std::string errMsg = "Unable to open log file '";
		errMsg += fileName;
		errMsg += "' for writing: ";
		errMsg += Util::ErrnoToString();
		throw Exception(std::move(errMsg), "Log");
	}

	//logWrite("--- LOG BEGIN ---\n");

	initColorize();
}

Log::~Log()
{
	if (hasFile())
	{
		//logWrite("---  LOG END  ---\n");
		close(m_fd);
	}
}

void Log::logV(EType msgType, const char *format, va_list args)
{
	// drop message with invalid type
	if (msgType < ALWAYS || msgType > DEBUG)
	{
		return;
	}

	// drop message with type above current verbosity level
	switch (msgType)
	{
		case ALWAYS:
		case ERROR:
		case WARNING:
		{
			if (m_verbosity < VERBOSITY_LOW)
			{
				return;
			}
			break;
		}
		case NOTICE:
		{
			if (m_verbosity < VERBOSITY_NORMAL)
			{
				return;
			}
			break;
		}
		case INFO:
		{
			if (m_verbosity < VERBOSITY_HIGH)
			{
				return;
			}
			break;
		}
		case DEBUG:
		{
			if (m_verbosity < VERBOSITY_DEBUG)
			{
				return;
			}
			break;
		}
	}

	CharBuffer<2048> msg;

	KString colorSuffix;
	if (m_isColorizeEnabled)
	{
		msg += COLOR_PREFIX_TABLE[msgType];
		colorSuffix = COLOR_SUFFIX_TABLE[msgType];
	}

	switch (m_style)
	{
		case STYLE_SIMPLE:
		{
			msg += PREFIX_TABLE_SIMPLE[msgType];

			break;
		}
		case STYLE_PRINTK:
		{
			msg += PREFIX_TABLE_PRINTK[msgType];

			break;
		}
		case STYLE_TIMESTAMP:
		{
			const std::string unixTimeString = gPlatform->getCurrentUnixTime().toString();

			msg += '[';
			msg += unixTimeString;
			msg += ']';
			msg += ' ';

			msg += PREFIX_TABLE_SIMPLE[msgType];

			break;
		}
		case STYLE_DATETIME_UTC:
		{
			const std::string dateTimeString = gPlatform->getCurrentDateTime(DateTime::UTC).toString();

			msg += '[';
			msg += dateTimeString;
			msg += ']';
			msg += ' ';

			msg += PREFIX_TABLE_SIMPLE[msgType];

			break;
		}
		case STYLE_DATETIME_LOCAL:
		{
			const std::string dateTimeString = gPlatform->getCurrentDateTime(DateTime::LOCAL).toString();

			msg += '[';
			msg += dateTimeString;
			msg += ']';
			msg += ' ';

			msg += PREFIX_TABLE_SIMPLE[msgType];

			break;
		}
	}

	const size_t prefixLength = msg.getDataLength();
	const size_t suffixLength = colorSuffix.length() + 1;  // newline character

	// build message content
	int status = msg.append_vf(format, args);  // too long message is truncated

	if (msg.getAvailableLength() < suffixLength)
	{
		// the log message is too long to fit in the stack buffer :(
		// so we need to allocate heap buffer of required size and build the message there

		const size_t contentLength = (status > 0) ? status : 0;
		const size_t totalLength = prefixLength + contentLength + suffixLength + 1;  // terminating null byte

		// allocate the heap buffer
		std::unique_ptr<char[]> bufferGuard = std::make_unique<char[]>(totalLength);

		char *buffer = bufferGuard.get();
		size_t pos = 0;

		// copy existing message prefix from the stack buffer
		std::memcpy(buffer+pos, msg.get().c_str(), prefixLength);
		pos += prefixLength;

		// build message content
		status = std::vsnprintf(buffer+pos, contentLength+1, format, args);
		if (status > 0)
		{
			const unsigned int length = status;
			pos += (length > contentLength) ? contentLength : length;
		}

		// add message suffix
		if (!colorSuffix.empty())
		{
			std::memcpy(buffer+pos, colorSuffix.c_str(), colorSuffix.length());
			pos += colorSuffix.length();
		}
		buffer[pos] = '\n';
		pos++;

		// add terminating null byte
		buffer[pos] = '\0';

		// send message to the log
		logWrite(KString(buffer, pos));
	}
	else
	{
		// add message suffix
		msg += colorSuffix;
		msg += '\n';

		// send message to the log
		logWrite(msg.get());
	}
}

void Log::setVerbosity(EVerbosity verbosity)
{
	m_verbosity = verbosity;
}

void Log::logWrite(const KString & data)
{
	// this should always succeed
	if (write(m_fd, data.c_str(), data.length()) < 0)  // write syscall
	{
		throw std::system_error(errno, std::system_category(), "Log write failed");
	}
}

void Log::initColorize()
{
	if (m_colorize == COLORIZE_ALWAYS || (m_colorize == COLORIZE_AUTO && isatty(m_fd) == 1))
	{
		m_isColorizeEnabled = true;
	}
	else
	{
		m_isColorizeEnabled = false;
	}
}
