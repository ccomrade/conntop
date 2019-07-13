/**
 * @file
 * @brief Screen class.
 */

#pragma once

#include <curses.h>
#include <vector>
#include <utility>  // std::pair

#include "IScreen.hpp"
#include "KString.hpp"
#include "Compiler.hpp"  // COMPILER_PRINTF_ARGS_CHECK

namespace ctp
{
	class Screen : public IScreen
	{
	public:
		struct Pos
		{
			int x;
			int y;

			constexpr Pos( int X = 0, int Y = 0 )
			: x(X),
			  y(Y)
			{
			}
		};

		using Size = Pos;

	private:
		WINDOW *m_window;
		Screen *m_parentScreen;
		Size m_minSize;
		Size m_maxSize;
		std::pair<Pos, Pos> m_screenRect;
		Pos m_screenRectScrollPos;
		bool m_isRefreshRequired;
		std::vector<Screen*> m_dialogStack;

		Size calculateSize();
		void updateScreenRect();
		bool scrollScreenRectX( int amount );
		bool scrollScreenRectY( int amount );

	protected:
		Screen( const Size & minSize, const Size & maxSize, Screen *pParentScreen = nullptr );

		Size getMinSize() const
		{
			return m_minSize;
		}

		void setMinSize( const Size & minSize );

		Size getMaxSize() const
		{
			return m_maxSize;
		}

		void setMaxSize( const Size & maxSize );

		Size getSize() const
		{
			Size size;
			getmaxyx( m_window, size.y, size.x );
			return size;
		}

		int getWidth() const
		{
			return getmaxx( m_window );
		}

		int getHeight() const
		{
			return getmaxy( m_window );
		}

		Size getTerminalSize() const
		{
			Size size;
			getmaxyx( stdscr, size.y, size.x );
			return size;
		}

		int getTerminalWidth() const
		{
			return getmaxx( stdscr );
		}

		int getTerminalHeight() const
		{
			return getmaxy( stdscr );
		}

		void windowContentModified()
		{
			m_isRefreshRequired = true;
		}

		void enableAttr( int attr )
		{
			wattron( m_window, attr );
		}

		void disableAttr( int attr )
		{
			wattroff( m_window, attr );
		}

		void resetAttr();

		Pos getPos()
		{
			Pos pos;
			getyx( m_window, pos.y, pos.x );
			return pos;
		}

		void setPos( const Pos & pos )
		{
			wmove( m_window, pos.y, pos.x );
		}

		void setPos( int posX, int posY )
		{
			wmove( m_window, posY, posX );
		}

		void writeString( const char *string, int maxLength = -1 )
		{
			waddnstr( m_window, string, maxLength );
			m_isRefreshRequired = true;
		}

		void writeString( const KString & string, int maxLength = -1 )
		{
			waddnstr( m_window, string.c_str(), maxLength );
			m_isRefreshRequired = true;
		}

		void writeChar( int ch )
		{
			waddch( m_window, ch );
			m_isRefreshRequired = true;
		}

		int writef( const char *format, ... ) COMPILER_PRINTF_ARGS_CHECK(2,3);

		bool hasParentScreen() const
		{
			return m_parentScreen != nullptr;
		}

		Screen *getParentScreen()
		{
			return m_parentScreen;
		}

		virtual void handleResize() = 0;
		virtual bool handleKey( int ch ) = 0;

	public:
		virtual ~Screen();

		bool isRefreshRequired() override
		{
			if ( m_isRefreshRequired )
			{
				return true;
			}

			for ( Screen *dialog : m_dialogStack )
			{
				if ( dialog->isRefreshRequired() )
				{
					return true;
				}
			}

			return false;
		}

		void refresh() override;
		void invalidate() override;
		void onResize() override;
		bool onKey( int ch ) override;

		void pushDialog( Screen *pDialogScreen )
		{
			if ( hasDialog( pDialogScreen ) )
			{
				return;
			}

			m_dialogStack.push_back( pDialogScreen );
			pDialogScreen->onResize();
			invalidate();
			for ( Screen *dialog : m_dialogStack )
			{
				dialog->invalidate();
			}
		}

		void removeDialog( const Screen *pDialogScreen )
		{
			bool removed = false;
			for ( auto it = m_dialogStack.begin(); it != m_dialogStack.end(); )
			{
				if ( *it == pDialogScreen )
				{
					it = m_dialogStack.erase( it );
					removed = true;
				}
				else
				{
					if ( removed )
					{
						(*it)->invalidate();
					}
					++it;
				}
			}

			if ( removed )
			{
				invalidate();
			}
		}

		bool hasDialog( const Screen *pDialogScreen ) const
		{
			for ( const Screen *dialog : m_dialogStack )
			{
				if ( dialog == pDialogScreen )
				{
					return true;
				}
			}
			return false;
		}

		WINDOW *getWindow()
		{
			m_isRefreshRequired = true;
			return m_window;
		}
	};

	inline bool operator==( const Screen::Pos & a, const Screen::Pos & b )
	{
		return a.x == b.x && a.y == b.y;
	}

	inline bool operator!=( const Screen::Pos & a, const Screen::Pos & b )
	{
		return ! (a == b);
	}
}
