#pragma once

#include "System/Log.h"
#include "System/Reactor.h"

class App
{
	Reactor m_reactor;

public:
	App() = default;

	void Run();
};
