/**
 * @file
 * @brief Conntrack class.
 */

#pragma once

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#include <deque>

#include "Connection.hpp"

class ConntrackEvent
{
public:
	enum EType
	{
		NEW_CONNECTION,
		REMOVED_CONNECTION
	};

private:
	EType m_type;
	Connection m_connection;

public:
	ConntrackEvent(EType type, const Connection & connection)
	: m_type(type),
	  m_connection(connection)
	{
	}

	EType getType() const
	{
		return m_type;
	}

	const Connection & getConnection() const
	{
		return m_connection;
	}
};

class ConntrackSocket
{
	nfct_handle *m_socket;

public:
	ConntrackSocket(unsigned int events = 0);
	~ConntrackSocket();

	nfct_handle *get()
	{
		return m_socket;
	}

	int getFD()
	{
		return nfct_fd(m_socket);
	}
};

class Conntrack
{
	std::deque<ConntrackEvent> m_eventQueue;
	ConntrackSocket m_querySocket;
	ConntrackSocket m_eventSocket;
	IConnectionUpdateCallback *m_callback;
	bool m_isInitialized;
	bool m_isRefillRequired;
	bool m_isPaused;

	void handleQuery(nf_conntrack *ct);
	void handleEvent(nf_conntrack *ct, ConntrackEvent::EType eventType);

	static int QueryCallback(nf_conntrack_msg_type type, nf_conntrack *ct, void *param);
	static int EventCallback(nf_conntrack_msg_type type, nf_conntrack *ct, void *param);
	static void EventPollHandler(int flags, void *param);

public:
	Conntrack();
	~Conntrack();

	void init(IConnectionUpdateCallback *callback);

	void onUpdate();

	bool isPaused() const
	{
		return m_isPaused;
	}

	void setPaused(bool paused)
	{
		if (m_isPaused != paused)
		{
			m_isPaused = paused;
			m_isRefillRequired = true;
		}
	}
};
