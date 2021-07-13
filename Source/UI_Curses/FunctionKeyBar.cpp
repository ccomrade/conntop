/**
 * @file
 * @brief Implementation of FunctionKeyBar class.
 */

#include <curses.h>

#include "FunctionKeyBar.hpp"
#include "ColorSystem.hpp"
#include "CursesEvent.hpp"
#include "App.hpp"
#include "Client.hpp"
#include "Screen.hpp"

namespace ctp
{
	FunctionKeyBar::FunctionKeyBar(Screen *parent)
	: m_parent(parent),
	  m_functionKeys{}
	{
		for (unsigned int i = 0; i < m_functionKeys.size(); i++)
		{
			restoreKey(static_cast<EKey>(i));
		}
	}

	void FunctionKeyBar::restoreKey(EKey key)
	{
		switch (key)
		{
			case F1:
			{
				initKey(F1, "Help");
				break;
			}
			case F7:
			{
				initKey(F7, "Prev");
				break;
			}
			case F8:
			{
				initKey(F8, "Next");
				break;
			}
			case F10:
			{
				initKey(F10, "Quit");
				break;
			}
			default:
			{
				m_functionKeys[key].isEnabled = false;
				break;
			}
		}
	}

	bool FunctionKeyBar::handlePressedKey(int ch)
	{
		switch (ch)
		{
			case KEY_F(1):
			{
				if (isKeyEnabled(F1) && !isKeyCustom(F1))
				{
					gApp->getEventSystem()->dispatch<CursesEvent>(CursesEvent::SHOW_HELP);
					return true;
				}
				break;
			}
			case KEY_F(7):
			case KEY_SLEFT:
			{
				if (isKeyEnabled(F7) && !isKeyCustom(F7))
				{
					gApp->getEventSystem()->dispatch<CursesEvent>(CursesEvent::PREV_SCREEN);
					return true;
				}
				break;
			}
			case KEY_F(8):
			case KEY_SRIGHT:
			{
				if (isKeyEnabled(F8) && !isKeyCustom(F8))
				{
					gApp->getEventSystem()->dispatch<CursesEvent>(CursesEvent::NEXT_SCREEN);
					return true;
				}
				break;
			}
			case KEY_F(10):
			{
				if (isKeyEnabled(F10) && !isKeyCustom(F10))
				{
					if (gApp->isClient() && gApp->getClient()->isConnected())
					{
						gApp->getClient()->disconnect();
					}
					else
					{
						gApp->quit();
					}
					return true;
				}
				break;
			}
		}

		return false;
	}

	void FunctionKeyBar::draw()
	{
		WINDOW *window = m_parent->getWindow();

		int windowHeight, windowWidth;
		getmaxyx(window, windowHeight, windowWidth);
		if (windowHeight == 0 || windowWidth == 0)
		{
			return;
		}

		const int attrLabel = gColorSystem->getAttr(ColorSystem::ATTR_FUNCTION_KEY_BAR_LABEL);
		const int attrName = gColorSystem->getAttr(ColorSystem::ATTR_FUNCTION_KEY_BAR_NAME);

		wmove(window, windowHeight-1, 0);
		unsigned int remainingSpace = windowWidth;

		for (unsigned int i = 0; i < m_functionKeys.size(); i++)
		{
			unsigned int space;

			wattron(window, attrLabel);

			if (i < 9)
			{
				space = (remainingSpace > 2) ? 2 : remainingSpace;
			}
			else
			{
				space = (remainingSpace > 3) ? 3 : remainingSpace;
			}

			if (space >= 1)
			{
				waddch(window, 'F');
				waddnstr(window, std::to_string(i+1).c_str(), space-1);
			}

			remainingSpace -= space;

			wattroff(window, attrLabel);
			wattron(window, attrName);

			space = (remainingSpace > 6) ? 6 : remainingSpace;

			unsigned int nameLength = 0;
			if (m_functionKeys[i].isEnabled)
			{
				KString name = m_functionKeys[i].name;
				nameLength = name.length();
				waddnstr(window, name.c_str(), space);
			}

			for (int freeSpace = space - nameLength; freeSpace > 0; freeSpace--)
			{
				waddch(window, ' ');
			}

			remainingSpace -= space;

			wattroff(window, attrName);
		}

		if (remainingSpace > 0)
		{
			wattron(window, attrName);

			for (; remainingSpace > 0; remainingSpace--)
			{
				waddch(window, ' ');
			}

			wattroff(window, attrName);
		}
	}
}
