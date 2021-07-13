/**
 * @file
 * @brief Curses class.
 */

#pragma once

#include <array>

#include "Log.hpp"
#include "CursesEvent.hpp"
#include "ClientEvent.hpp"
#include "IEventCallback.hpp"
#include "Screens/ScreenHelp.hpp"
#include "Screens/ScreenConnectionList.hpp"

struct IScreen;

class CursesTerminalGuard
{
	Log::EVerbosity m_logVerbosity;

	void restoreTerminal();
	void restoreLog();

public:
	CursesTerminalGuard();
	~CursesTerminalGuard();
};

class Curses : public IEventCallback<CursesEvent>, public IEventCallback<ClientEvent>
{
	CursesTerminalGuard m_terminalGuard;
	ScreenConnectionList m_screenConnectionList;
	std::array<IScreen*, 1> m_screens;
	IScreen *m_currentScreen;
	IScreen *m_oldScreen;
	ScreenHelp m_screenHelp;

	void resizeScreen();
	void processInput();
	void nextScreen();
	void prevScreen();
	void showHelp();
	void closeHelp();
	int getTerminalSize(int & lines, int & columns);

	static void KeyboardPollHandler(int flags, void *param);

public:
	Curses();
	~Curses();

	void init();

	void onEvent(const CursesEvent & event) override;
	void onEvent(const ClientEvent & event) override;

	ScreenConnectionList & getConnectionListScreen()
	{
		return m_screenConnectionList;
	}

	bool isCurrentScreen(const IScreen & screen) const
	{
		return (m_currentScreen == &screen);
	}

	void refreshScreen(bool redraw = false);
};
