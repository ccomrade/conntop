#pragma once

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <vector>

#include "App/Connection.h"

class Conntrack
{
	nfct_handle* m_handle = nullptr;
	std::vector<Connection> m_connections;

	void UpdateCallback(nf_conntrack_msg_type type, nf_conntrack* ct);
	static int UpdateCallbackWrapper(nf_conntrack_msg_type type, nf_conntrack* ct, void* data);

	static bool ToConnection(nf_conntrack* ct, Connection& result);

public:
	Conntrack();
	~Conntrack();

	void Update();

	const std::vector<Connection>& GetConnections() const
	{
		return m_connections;
	}
};
