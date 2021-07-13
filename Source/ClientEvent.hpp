/**
 * @file
 * @brief ClientEvent class.
 */

#pragma once

#include "Events.hpp"

namespace ctp
{
	struct ClientEvent
	{
		static constexpr int ID = EGlobalEventID::CLIENT_EVENT;

		enum EType
		{
			DISCONNECTED,
			CONNECT_STARTED,
			DISCONNECT_STARTED,
			CONNECTION_ESTABLISHED,
			SESSION_ESTABLISHED,
			SERVER_UPDATE_TICK,
			NEW_DATA_AVAILABLE,
			SYNC_STATE_CHANGED
		};

	private:
		EType m_type;

	public:
		ClientEvent(EType type)
		: m_type(type)
		{
		}

		EType getType() const
		{
			return m_type;
		}
	};
}
