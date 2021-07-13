/**
 * @file
 * @brief Application entry point on Unix platform.
 */

#include <iostream>
#include <new>

#include "CmdLine.hpp"
#include "Platform.hpp"
#include "Log.hpp"
#include "App.hpp"
#include "Exception.hpp"
#include "Thread.hpp"
#include "Util.hpp"
#include "Compiler.hpp"
#include "Version.hpp"

namespace ctp
{
	class MemoryAllocationException : public std::bad_alloc
	{
	public:
		MemoryAllocationException()
		{
			if (gLog)
			{
				gLog->error("Memory allocation failed in %s thread", Thread::GetCurrentThreadName().c_str());
			}
		}

		const char *what() const noexcept override
		{
			return "Memory allocation failed";
		}

		static void Init()
		{
			auto NewHandler = []() -> void
			{
				std::set_new_handler(nullptr);
				throw MemoryAllocationException();
			};

			std::set_new_handler(NewHandler);
		}
	};

	struct PlatformInitGuard
	{
		PlatformInitGuard()
		{
			gPlatform->start();
		}

		~PlatformInitGuard()
		{
			Util::LogMemoryUsage();
			gPlatform->stop();
		}
	};

	static void CreateGlobalCmdLine(int argc, char *argv[])
	{
		gCmdLine = new CmdLine(argc, argv);

		auto DestroyGlobalCmdLine = []() -> void
		{
			delete gCmdLine;
			gCmdLine = nullptr;
		};

		std::atexit(DestroyGlobalCmdLine);
	}

	static void CreateGlobalPlatform()
	{
		gPlatform = new Platform();

		auto DestroyGlobalPlatform = []() -> void
		{
			delete gPlatform;
			gPlatform = nullptr;
		};

		std::atexit(DestroyGlobalPlatform);
	}

	static void CreateGlobalLog()
	{
		// default settings
		Log::EVerbosity verbosity = Log::VERBOSITY_LOW;
		Log::EColorize colorize = Log::COLORIZE_NEVER;
		Log::EStyle style = Log::STYLE_SIMPLE;

		if (CmdLineArg *verbosityArg = gCmdLine->getArg("verbose"))
		{
			switch (verbosityArg->getCount())
			{
				case 0:
				{
					verbosity = Log::VERBOSITY_LOW;
					break;
				}
				case 1:
				{
					verbosity = Log::VERBOSITY_NORMAL;
					break;
				}
				case 2:
				{
					verbosity = Log::VERBOSITY_HIGH;
					break;
				}
				default:
				{
					verbosity = Log::VERBOSITY_DEBUG;
					break;
				}
			}
		}

		if (CmdLineArg *colorizeArg = gCmdLine->getArg("log-color"))
		{
			KString colorizeValue = colorizeArg->getValue();
			if (colorizeValue.empty())
			{
				// value of --log-color is optional
				colorize = Log::COLORIZE_ALWAYS;
			}
			else if (colorizeValue == "never")
			{
				colorize = Log::COLORIZE_NEVER;
			}
			else if (colorizeValue == "auto")
			{
				colorize = Log::COLORIZE_AUTO;
			}
			else if (colorizeValue == "always")
			{
				colorize = Log::COLORIZE_ALWAYS;
			}
			else
			{
				std::string errMsg = "Invalid value '";
				errMsg += colorizeValue;
				errMsg += "' of '--log-color'";
				throw Exception(std::move(errMsg), "Main");
			}
		}

		if (CmdLineArg *styleArg = gCmdLine->getArg("log-style"))
		{
			KString styleValue = styleArg->getValue();
			if (styleValue == "simple")
			{
				style = Log::STYLE_SIMPLE;
			}
			else if (styleValue == "printk")
			{
				style = Log::STYLE_PRINTK;
			}
			else if (styleValue == "timestamp")
			{
				style = Log::STYLE_TIMESTAMP;
			}
			else if (styleValue == "datetime")
			{
				style = Log::STYLE_DATETIME_LOCAL;
			}
			else
			{
				std::string errMsg = "Invalid value '";
				errMsg += styleValue;
				errMsg += "' of '--log-style'";
				throw Exception(std::move(errMsg), "Main");
			}
		}

		if (style == Log::STYLE_DATETIME_LOCAL && gCmdLine->hasArg("log-utc"))
		{
			style = Log::STYLE_DATETIME_UTC;
		}

		if (CmdLineArg *fileArg = gCmdLine->getArg("log-file"))
		{
			bool clear = gCmdLine->hasArg("log-clear");
			gLog = new Log(verbosity, colorize, style, fileArg->getValue(), clear);
		}
		else
		{
			gLog = new Log(verbosity, colorize, style);
		}

		auto DestroyGlobalLog = []() -> void
		{
			delete gLog;
			gLog = nullptr;
		};

		std::atexit(DestroyGlobalLog);
	}

	static void ShowHelp()
	{
		std::cout << "Usage: " CONNTOP_APP_NAME " [OPTIONS]\n\n"
		          << CmdLine::CreateOptionsList() << std::endl;
	}

	static void ShowVersion()
	{
		std::cout << VERSION << "\n"
		          << COPYRIGHT_NOTICE << "\n\n"
		          << LICENSE_NOTICE << "\n\n"
		          << "Compiled by " << COMPILER_NAME_VERSION << std::endl;
	}

	static void ShowError(const KString & errMsg)
	{
		std::cerr << "ERROR: " << errMsg << std::endl;
	}

	static void ShowCmdLineError(const KString & appName, const KString & errMsg)
	{
		std::cerr << errMsg << "\n"
		          << "Try '" << appName << " -h' or '" << appName << " --help' for more information." << std::endl;
	}
}

int main(int argc, char *argv[])
{
	using namespace ctp;

	Thread::SetCurrentThreadName("Main");

	// enable custom replacement of std::bad_alloc
	MemoryAllocationException::Init();

	try
	{
		// parse command line
		CreateGlobalCmdLine(argc, argv);
	}
	catch (const CmdLineParseException & e)
	{
		ShowCmdLineError(argv[0], e.getString());
		return 2;
	}

	if (gCmdLine->hasArg("help"))
	{
		ShowHelp();
		return 0;
	}

	if (gCmdLine->hasArg("version"))
	{
		ShowVersion();
		return 0;
	}

	try
	{
		CreateGlobalPlatform();
		CreateGlobalLog();

		// create gApp
		App app;

		gLog->debug("[Main] Global environment created");

		// init platform
		PlatformInitGuard platformInitGuard;

		// start the application and enter update loop
		gApp->launch();
	}
	catch (const Exception & e)
	{
		if (!e.wasLogAvailable() || !gLog || gLog->hasFile() || gLog->getVerbosity() == Log::VERBOSITY_DISABLED)
		{
			ShowError(e.getString());
		}
		return 1;
	}

	return 0;
}
