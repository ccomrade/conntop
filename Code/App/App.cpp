#include "App.h"

void App::Run()
{
	m_reactor.Init();

	Log::Error("Error");
	Log::Warning("Warning");
	Log::Notice("Notice");
	Log::Info("Info");
	Log::Debug("Debug");

	m_reactor.EventLoop();
}
