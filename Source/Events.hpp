/**
 * @file
 * @brief Global event IDs and common events.
 */

#pragma once

#include "KString.hpp"

namespace ctp
{
	namespace EGlobalEventID
	{
		enum EID
		{
			UPDATE_EVENT,
			CLIENT_EVENT,

			// private events
			APP_INTERNAL_EVENT,
			RESOLVER_INTERNAL_EVENT,
			POLL_SYSTEM_INTERNAL_EVENT,
			UI_CURSES_INTERNAL_EVENT
		};

		KString ToString(int id);
	}

	struct UpdateEvent
	{
		static constexpr int ID = EGlobalEventID::UPDATE_EVENT;
	};
}
