/**
 * @file
 * @brief Global environment.
 */

#pragma once

namespace ctp
{
	// forward declarations
	class CmdLine;
	class Platform;
	class Log;
	class App;
}

// global environment
extern ctp::CmdLine *gCmdLine;
extern ctp::Platform *gPlatform;
extern ctp::Log *gLog;
extern ctp::App *gApp;
