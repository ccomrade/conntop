/**
 * @file
 * @brief CursesEvent class.
 */

#pragma once

#include "Events.hpp"

namespace ctp
{
	struct CursesEvent
	{
		static constexpr int ID = EGlobalEventID::UI_CURSES_INTERNAL_EVENT;

		enum EType
		{
			TERMINAL_RESIZED,
			NEXT_SCREEN,
			PREV_SCREEN,
			SHOW_HELP,
			CLOSE_HELP
		};

	private:
		EType m_type;

	public:
		CursesEvent(EType type)
		: m_type(type)
		{
		}

		EType getType() const
		{
			return m_type;
		}
	};
}
