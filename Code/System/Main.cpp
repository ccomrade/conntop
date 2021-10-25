#include <stdexcept>

#include "App/App.h"

#include "CmdLine.h"
#include "Log.h"

namespace
{
	void ParseCmdLine(int argc, char** argv)
	{
		CmdLine cmdLine(argc, argv);

		// TODO
		Log::config.verbosity = static_cast<int>(cmdLine.CountAndPopOption("v", "verbose"));

		// make sure there are no remaining options
		for (const CmdLine::Option& option : cmdLine.GetOptions())
		{
			throw std::runtime_error("Unsupported option '" + option.GetPrettyName() + "'");
		}

		// make sure there are no remaining operands
		for (const std::string_view& operand : cmdLine.GetOperands())
		{
			throw std::runtime_error("Unsupported operand '" + std::string(operand) + "'");
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
		LOG_ERROR(error.what());
		return 2;
	}

	try
	{
		App().Run();
	}
	catch (const std::runtime_error& error)
	{
		LOG_ERROR(error.what());
		return 1;
	}

	return 0;
}
