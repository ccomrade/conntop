#pragma once

#include <memory>

class Conntrack;

class Probe
{
	std::unique_ptr<Conntrack> m_conntrack;

public:
	Probe() = default;

	void Init();
};
