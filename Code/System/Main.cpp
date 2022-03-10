#include <stdexcept>

#include "App/App.h"

#include "CmdLine.h"
#include "Log.h"

namespace
{
	void ParseCmdLine(int argc, char** argv)
	{
		CmdLine cmdLine(argc, argv);

		const unsigned int verboseCount = cmdLine.CountAndPopOption("v", "verbose");

		Log::GetInstance().SetVerbosity(static_cast<Log::Verbosity>(verboseCount));

		// make sure there are no remaining options
		for (const CmdLine::Option& option : cmdLine.options)
		{
			throw std::runtime_error("Unknown option '" + CmdLine::AddHyphens(option.name) + "'");
		}

		// make sure there are no remaining operands
		for (const std::string_view& operand : cmdLine.operands)
		{
			throw std::runtime_error("Unknown operand '" + std::string(operand) + "'");
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		ParseCmdLine(argc, argv);
	}
	catch (const std::runtime_error& error)
	{
		Log::Error("{}", error.what());
		return 2;
	}

	try
	{
		App().Run();
	}
	catch (const std::runtime_error& error)
	{
		Log::Error("{}", error.what());
		return 1;
	}

	return 0;
}
