#include "Probe.h"
#include "Conntrack.h"

void Probe::Init()
{
	m_conntrack = std::make_unique<Conntrack>();
}
