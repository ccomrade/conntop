/**
 * @file
 * @brief ScreenConnectionList class.
 */

#pragma once

#include "Screen.hpp"
#include "FunctionKeyBar.hpp"
#include "DialogConnectionDetails.hpp"
#include "DialogConnectionColumns.hpp"
#include "DialogConnectionSort.hpp"
#include "ClientEvent.hpp"

namespace ctp
{
	class ConnectionListUpdate;

	class ScreenConnectionList : public Screen
	{
		static constexpr unsigned int STATUS_HEADER_HEIGHT = 4;
		static constexpr unsigned int MINIMUM_HEIGHT = STATUS_HEADER_HEIGHT + 2;  // list header + function key bar
		static constexpr unsigned int MINIMUM_WIDTH = 80;

		FunctionKeyBar m_keyBar;
		DialogConnectionDetails m_dialogDetails;
		DialogConnectionColumns m_dialogColumns;
		DialogConnectionSort m_dialogSortMode;
		unsigned int m_columnsTotalWidth;
		unsigned int m_currentSize;
		unsigned int m_connectionCount;
		unsigned int m_scrollOffset;
		int m_resolvedPercentage;
		bool m_showCursor;
		bool m_showHostname;
		bool m_showPortname;
		bool m_pausedState;
		bool m_syncState;

		void drawStatic();
		void drawListHeader();
		void drawConnectionCount();
		void drawResolvedPercentage();
		void drawScrollOffset();
		void drawHostnameState();
		void drawPortnameState();
		void drawPausedState();
		void drawSyncState();
		void drawCollectorInfo();
		void drawListEntry( const ConnectionListUpdate & update );
		void drawDataLeft( const KString & data, unsigned int width, int attr );
		void drawDataRight( const KString & data, unsigned int width, int attr );
		void drawDataEmpty( unsigned int width, int attr );
		void clearList();
		void fillEmpty( int count );
		void fillEmpty();
		void setMinWidth();

		unsigned int getListSize() const
		{
			return getHeight() - MINIMUM_HEIGHT;
		}

		unsigned int getListBeginPos() const
		{
			return STATUS_HEADER_HEIGHT + 1;
		}

		void handleResize() override;
		bool handleKey( int ch ) override;

	public:
		ScreenConnectionList();

		void updateList();
		void updateColumns();
		void updateSortMode();
		void updateClientInfo( const ClientEvent & event );
	};
}
