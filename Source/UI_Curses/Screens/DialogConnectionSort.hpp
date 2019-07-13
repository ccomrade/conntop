/**
 * @file
 * @brief DialogConnectionSort class.
 */

#pragma once

#include <array>

#include "KString.hpp"
#include "Screen.hpp"
#include "Connection.hpp"

namespace ctp
{
	class ScreenConnectionList;

	class DialogConnectionSort : public Screen
	{
	public:
		static constexpr unsigned int SORT_MODE_COUNT = 21;

		static const std::array<KString, SORT_MODE_COUNT> SORT_MODE_NAMES;

	private:
		EConnectionSortMode m_sortMode;
		unsigned int m_cursorPos;
		bool m_isSortAscending;
		bool m_checkBoxState;

		void applyConfig();
		void restoreConfig();
		void draw();
		void drawEntry( unsigned int index );
		void drawCheckbox();
		void fillEmpty( int count );
		void fillEmpty();

		void handleResize() override;
		bool handleKey( int ch ) override;

	public:
		DialogConnectionSort( ScreenConnectionList *parent );

		void open();
		void close( bool apply = false );

		EConnectionSortMode getSortMode() const
		{
			return m_sortMode;
		}

		bool isSortAscending() const
		{
			return m_isSortAscending;
		}
	};
}
