/**
 * @file
 * @brief Implementation of Curses class.
 */

#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <clocale>

#include "Curses.hpp"
#include "IScreen.hpp"
#include "ColorSystem.hpp"
#include "App.hpp"
#include "Exception.hpp"
#include "Util.hpp"

namespace ctp
{
	CursesTerminalGuard::CursesTerminalGuard()
	: m_logVerbosity(Log::VERBOSITY_DISABLED)
	{
		gLog->info( "[UI_Curses] Preparing to switch terminal into curses mode..." );

		const char *locale = std::setlocale( LC_ALL, "" );
		if ( locale == nullptr )
		{
			throw Exception( "Unable to set locale", "UI_Curses" );
		}

		gLog->info( "[UI_Curses] Locale: %s", locale );

		if ( ! gLog->hasFile() && gLog->getVerbosity() != Log::VERBOSITY_DISABLED )
		{
			gLog->notice( "[UI_Curses] Log is using terminal, disabling..." );
			m_logVerbosity = gLog->getVerbosity();
			gLog->setVerbosity( Log::VERBOSITY_DISABLED );
		}

		if ( initscr() == nullptr )  // terminal is restored automatically
		{
			restoreLog();
			throw Exception( "Unable to switch terminal into curses mode", "UI_Curses" );
		}

		gLog->notice( "[UI_Curses] Terminal succesfully switched to curses mode" );

		gLog->info( "[UI_Curses] Terminal size: %dx%d", COLS, LINES );

		if ( noecho() == ERR || cbreak() == ERR || nodelay( stdscr, TRUE ) == ERR || keypad( stdscr, TRUE ) == ERR )
		{
			restoreTerminal();
			restoreLog();
			throw Exception( "Unable to configure terminal", "UI_Curses" );
		}

		// the following functions always succeed
		nonl();
		curs_set( 0 );
		set_escdelay( 25 );  // 25ms seems to be safe
		leaveok( stdscr, TRUE );
		intrflush( stdscr, FALSE );

		try
		{
			ColorSystem::Init();
		}
		catch ( const Exception & e )
		{
			restoreTerminal();
			restoreLog();
			std::string errMsg = "Unable to initialize terminal colors: ";
			errMsg += e.getString();
			throw Exception( std::move( errMsg ), "UI_Curses" );
		}

		refresh();

		gLog->info( "[UI_Curses] Terminal initialized" );
	}

	CursesTerminalGuard::~CursesTerminalGuard()
	{
		restoreTerminal();
		restoreLog();
	}

	void CursesTerminalGuard::restoreTerminal()
	{
		gLog->notice( "[UI_Curses] Restoring terminal..." );
		endwin();
	}

	void CursesTerminalGuard::restoreLog()
	{
		if ( m_logVerbosity != Log::VERBOSITY_DISABLED )
		{
			gLog->setVerbosity( m_logVerbosity );
			gLog->notice( "[UI_Curses] Log re-enabled" );
		}
	}

	Curses::Curses()
	: m_terminalGuard(),
	  m_screenConnectionList(),
	  m_screens{{
		&m_screenConnectionList
	  }},
	  m_currentScreen(m_screens[0]),
	  m_oldScreen(nullptr),
	  m_screenHelp()
	{
		gApp->getEventSystem()->registerCallback<CursesEvent>( this );
		gApp->getEventSystem()->registerCallback<ClientEvent>( this );
		gApp->getPollSystem()->addFD( STDIN_FILENO, EPollFlags::INPUT, KeyboardPollHandler, this );

		refreshScreen();
	}

	Curses::~Curses()
	{
		gApp->getEventSystem()->removeCallback<CursesEvent>( this );
		gApp->getEventSystem()->removeCallback<ClientEvent>( this );
		gApp->getPollSystem()->removeFD( STDIN_FILENO );
	}

	void Curses::init()
	{
		// nothing to do here because initialization is done in constructor
		// UI is always created as the last subsystem, so the initialization can stay in constructor for now
	}

	void Curses::onEvent( const CursesEvent & event )
	{
		switch ( event.getType() )
		{
			case CursesEvent::TERMINAL_RESIZED:
			{
				resizeScreen();
				break;
			}
			case CursesEvent::NEXT_SCREEN:
			{
				nextScreen();
				break;
			}
			case CursesEvent::PREV_SCREEN:
			{
				prevScreen();
				break;
			}
			case CursesEvent::SHOW_HELP:
			{
				showHelp();
				break;
			}
			case CursesEvent::CLOSE_HELP:
			{
				closeHelp();
				break;
			}
		}
	}

	void Curses::onEvent( const ClientEvent & event )
	{
		m_screenConnectionList.updateClientInfo( event );
		refreshScreen();
	}

	void Curses::refreshScreen( bool redraw )
	{
		if ( redraw )
		{
			clear();
			wnoutrefresh( stdscr );
			m_currentScreen->invalidate();
			m_currentScreen->refresh();
			doupdate();
		}
		else if ( m_currentScreen->isRefreshRequired() )
		{
			m_currentScreen->refresh();
			doupdate();
		}
	}

	void Curses::resizeScreen()
	{
		int lines, columns;
		if ( getTerminalSize( lines, columns ) < 0 )
		{
			return;
		}

		if ( ! is_term_resized( lines, columns ) )
		{
			return;
		}

		if ( resize_term( lines, columns ) == ERR )
		{
			gLog->error( "[UI_Curses] Resize failed" );
			return;
		}

		gLog->info( "[UI_Curses] New terminal size: %dx%d", columns, lines );

		// resize all screens
		for ( IScreen *pScreen : m_screens )
		{
			pScreen->onResize();
		}
		m_screenHelp.onResize();

		// redraw screen content
		refreshScreen( true );
	}

	void Curses::processInput()
	{
		gLog->debug( "[UI_Curses] Keyboard input handler begin" );

		int ch;
		while ( (ch = getch()) != ERR )
		{
			gLog->debug( "[UI_Curses] Key %d '%s'", ch, keyname( ch ) );
			m_currentScreen->onKey( ch );
			refreshScreen();
		}

		gLog->debug( "[UI_Curses] Keyboard input handler end" );
	}

	void Curses::nextScreen()
	{
		const unsigned int lastIndex = m_screens.size() - 1;
		for ( unsigned int i = 0; i <= lastIndex; i++ )
		{
			if ( m_screens[i] == m_currentScreen )
			{
				m_currentScreen = (i == lastIndex) ? m_screens.front() : m_currentScreen + 1;
				refreshScreen( true );
				break;
			}
		}
	}

	void Curses::prevScreen()
	{
		for ( unsigned int i = 0; i < m_screens.size(); i++ )
		{
			if ( m_screens[i] == m_currentScreen )
			{
				m_currentScreen = (i == 0) ? m_screens.back() : m_currentScreen - 1;
				refreshScreen( true );
				break;
			}
		}
	}

	void Curses::showHelp()
	{
		if ( ! isCurrentScreen( m_screenHelp ) )
		{
			m_oldScreen = m_currentScreen;
			m_currentScreen = &m_screenHelp;
			refreshScreen( true );
		}
	}

	void Curses::closeHelp()
	{
		if ( isCurrentScreen( m_screenHelp ) )
		{
			m_currentScreen = m_oldScreen;
			refreshScreen( true );
		}
	}

	int Curses::getTerminalSize( int & lines, int & columns )
	{
		winsize size;
		if ( ioctl( STDOUT_FILENO, TIOCGWINSZ, &size ) != 0 )
		{
			gLog->error( "[UI_Curses] Unable to get terminal size: %s", Util::ErrnoToString().c_str() );
			return -1;
		}

		lines = size.ws_row;
		columns = size.ws_col;

		return 0;
	}

	void Curses::KeyboardPollHandler( int flags, void *param )
	{
		Curses *self = static_cast<Curses*>( param );

		if ( flags & EPollFlags::INPUT )
		{
			self->processInput();
		}

		if ( flags & EPollFlags::ERROR )
		{
			throw Exception( "Keyboard poll failed", "UI_Curses" );
		}

		gApp->getPollSystem()->resetFD( STDIN_FILENO, EPollFlags::INPUT );
	}
}
