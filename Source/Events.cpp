/**
 * @file
 * @brief Implementation of global event IDs and common events.
 */

#include "Events.hpp"

KString EGlobalEventID::ToString(int id)
{
	switch (static_cast<EGlobalEventID::EID>(id))
	{
		case UPDATE_EVENT:               return "UPDATE_EVENT";
		case CLIENT_EVENT:               return "CLIENT_EVENT";
		case APP_INTERNAL_EVENT:         return "APP_INTERNAL_EVENT";
		case RESOLVER_INTERNAL_EVENT:    return "RESOLVER_INTERNAL_EVENT";
		case POLL_SYSTEM_INTERNAL_EVENT: return "POLL_SYSTEM_INTERNAL_EVENT";
		case UI_CURSES_INTERNAL_EVENT:   return "UI_CURSES_INTERNAL_EVENT";
	}

	return "?";
}
