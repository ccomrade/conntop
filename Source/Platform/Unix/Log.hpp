/**
 * @file
 * @brief Log class for Unix platform.
 */

#pragma once

#include <cstdarg>

#include "GlobalEnvironment.hpp"
#include "KString.hpp"
#include "Compiler.hpp"

namespace ctp
{
	class Log
	{
	public:
		enum EType
		{
			ALWAYS,   //!< Informational message logged at all verbosity levels except VERBOSITY_DISABLED.
			ERROR,    //!< Error message.
			WARNING,  //!< Warning message.
			NOTICE,   //!< Notice message.
			INFO,     //!< Informational message.
			DEBUG     //!< Debug message.
		};

		enum EVerbosity
		{
			VERBOSITY_DISABLED,  //!< All log messages are disabled.
			VERBOSITY_LOW,       //!< ALWAYS, ERROR, WARNING
			VERBOSITY_NORMAL,    //!< ALWAYS, ERROR, WARNING, NOTICE
			VERBOSITY_HIGH,      //!< ALWAYS, ERROR, WARNING, NOTICE, INFO
			VERBOSITY_DEBUG      //!< All log messages are enabled.
		};

		enum EColorize
		{
			COLORIZE_NEVER,  //!< Never colorize log messages.
			COLORIZE_AUTO,   //!< Colorize log messages only if log output is terminal.
			COLORIZE_ALWAYS  //!< Always colorize log messages.
		};

		enum EStyle
		{
			STYLE_SIMPLE,         //!< Example: [INFO] This is informational message
			STYLE_PRINTK,         //!< Example: <6>This is informational message
			STYLE_TIMESTAMP,      //!< Example: [1550426046.177] [INFO] This is informational message
			STYLE_DATETIME_UTC,   //!< Example: [2019-02-17 17:57:33.824Z] [INFO] This is informational message
			STYLE_DATETIME_LOCAL  //!< Example: [2019-02-17 18:57:33.824+0100] [INFO] This is informational message
		};

	private:
		EVerbosity m_verbosity;
		EColorize m_colorize;
		EStyle m_style;
		KString m_fileName;
		bool m_isColorizeEnabled;
		int m_fd;

		void logWrite(const KString & data);
		void initColorize();

	public:
		Log(EVerbosity verbosity, EColorize colorize, EStyle style);
		Log(EVerbosity verbosity, EColorize colorize, EStyle style, const KString & fileName, bool clear = false);
		~Log();

		void logV(EType msgType, const char *format, va_list args);
		void setVerbosity(EVerbosity verbosity);

		EVerbosity getVerbosity() const
		{
			return m_verbosity;
		}

		EColorize getColorize() const
		{
			return m_colorize;
		}

		EStyle getStyle() const
		{
			return m_style;
		}

		KString getFileName() const
		{
			return m_fileName;
		}

		bool hasFile() const
		{
			return !m_fileName.empty();
		}

		bool isColorizeEnabled() const
		{
			return m_isColorizeEnabled;
		}

		bool isMsgEnabled(EType msgType) const
		{
			switch (msgType)
			{
				case ALWAYS:
				case ERROR:
				case WARNING:
				{
					if (m_verbosity >= VERBOSITY_LOW)
					{
						return true;
					}
					break;
				}
				case NOTICE:
				{
					if (m_verbosity >= VERBOSITY_NORMAL)
					{
						return true;
					}
					break;
				}
				case INFO:
				{
					if (m_verbosity >= VERBOSITY_HIGH)
					{
						return true;
					}
					break;
				}
				case DEBUG:
				{
					if (m_verbosity >= VERBOSITY_DEBUG)
					{
						return true;
					}
					break;
				}
			}

			return false;
		}

		void log(EType msgType, const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(3,4)
		{
			va_list args;
			va_start(args, format);
			logV(msgType, format, args);
			va_end(args);
		}

		void always(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(ALWAYS, format, args);
			va_end(args);
		}

		void error(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(ERROR, format, args);
			va_end(args);
		}

		void warning(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(WARNING, format, args);
			va_end(args);
		}

		void notice(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(NOTICE, format, args);
			va_end(args);
		}

		void info(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(INFO, format, args);
			va_end(args);
		}

		void debug(const char *format, ...) COMPILER_PRINTF_ARGS_CHECK(2,3)
		{
			va_list args;
			va_start(args, format);
			logV(DEBUG, format, args);
			va_end(args);
		}
	};
}
