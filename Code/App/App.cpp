#include "App.h"

void App::Run()
{
	m_reactor.Init();

	LOG_ERROR("Error");
	LOG_WARNING("Warning");
	LOG_NOTICE("Notice");
	LOG_INFO("Info");
	LOG_DEBUG("Debug");

	// event loop
	m_reactor.Run();
}
