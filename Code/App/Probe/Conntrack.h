#pragma once

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <memory>
#include <vector>

#include "App/Connection.h"

class Conntrack
{
	struct HandleDeleter
	{
		void operator()(nfct_handle* handle) const
		{
			nfct_close(handle);
		}
	};

	using Handle = std::unique_ptr<nfct_handle, HandleDeleter>;

	Handle m_handle;
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
