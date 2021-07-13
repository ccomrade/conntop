/**
 * @file
 * @brief Implementation of Screen class.
 */

#include <cstdio>  // std::vsnprintf
#include <cstdarg>
#include <cerrno>
#include <system_error>

#include "Screen.hpp"
#include "ColorSystem.hpp"

namespace ctp
{
	Screen::Screen(const Size & minSize, const Size & maxSize, Screen *pParentScreen)
	: m_window(nullptr),
	  m_parentScreen(pParentScreen),
	  m_minSize(minSize),
	  m_maxSize(maxSize),
	  m_screenRect(),
	  m_screenRectScrollPos(0, 0),
	  m_isRefreshRequired(false),
	  m_dialogStack()
	{
		const Size size = calculateSize();
		m_window = newpad(size.y, size.x);
		if (!m_window)
		{
			throw std::system_error(errno, std::system_category(), "Unable to create curses pad");
		}

		updateScreenRect();
	}

	Screen::~Screen()
	{
		delwin(m_window);
	}

	void Screen::refresh()
	{
		bool refreshed = false;

		if (m_isRefreshRequired)
		{
			m_isRefreshRequired = false;
			const Pos & minPos = m_screenRect.first;
			const Pos & maxPos = m_screenRect.second;
			const Pos & scrollPos = m_screenRectScrollPos;
			pnoutrefresh(m_window, scrollPos.y, scrollPos.x, minPos.y, minPos.x, maxPos.y, maxPos.x);
			refreshed = true;
		}

		for (Screen *dialog : m_dialogStack)
		{
			if (refreshed)
			{
				dialog->invalidate();
				dialog->refresh();
			}
			else if (dialog->isRefreshRequired())
			{
				dialog->refresh();
				refreshed = true;
			}
		}
	}

	void Screen::invalidate()
	{
		touchwin(m_window);
		m_isRefreshRequired = true;
	}

	void Screen::onResize()
	{
		const Size newSize = calculateSize();
		if (newSize != getSize())
		{
			werase(m_window);
			wresize(m_window, newSize.y, newSize.x);
			handleResize();
		}

		updateScreenRect();

		for (Screen *dialog : m_dialogStack)
		{
			dialog->onResize();
		}
	}

	bool Screen::onKey(int ch)
	{
		for (int i = m_dialogStack.size()-1; i >= 0; i--)
		{
			if (m_dialogStack[i]->onKey(ch))
			{
				return true;
			}
		}

		switch (ch)
		{
			case KEY_UP:
			{
				if (scrollScreenRectY(-1))
				{
					invalidate();
					return true;
				}
				break;
			}
			case KEY_DOWN:
			{
				if (scrollScreenRectY(1))
				{
					invalidate();
					return true;
				}
				break;
			}
			case KEY_LEFT:
			{
				if (scrollScreenRectX(-1))
				{
					invalidate();
					return true;
				}
				break;
			}
			case KEY_RIGHT:
			{
				if (scrollScreenRectX(1))
				{
					invalidate();
					return true;
				}
				break;
			}
		}

		return handleKey(ch);
	}

	void Screen::setMinSize(const Size & minSize)
	{
		if (minSize == m_minSize)
		{
			return;
		}

		m_minSize = minSize;
		const Size newSize = calculateSize();
		if (newSize != getSize())
		{
			werase(m_window);
			wresize(m_window, newSize.y, newSize.x);
			invalidate();
			updateScreenRect();
			handleResize();
		}
	}

	void Screen::setMaxSize(const Size & maxSize)
	{
		if (maxSize == m_maxSize)
		{
			return;
		}

		m_maxSize = maxSize;
		const Size newSize = calculateSize();
		if (newSize != getSize())
		{
			werase(m_window);
			wresize(m_window, newSize.y, newSize.x);
			invalidate();
			updateScreenRect();
			handleResize();
		}
	}

	void Screen::resetAttr()
	{
		wattrset(m_window, gColorSystem->getAttr(ColorSystem::ATTR_DEFAULT));
	}

	int Screen::writef(const char *format, ...)
	{
		char buffer[4096];
		va_list args;
		va_start(args, format);
		int status = std::vsnprintf(buffer, sizeof buffer, format, args);
		va_end(args);

		if (status < 0)
		{
			return -1;
		}

		waddstr(m_window, buffer);
		m_isRefreshRequired = true;

		return status;
	}

	Screen::Size Screen::calculateSize()
	{
		Size size = getTerminalSize();

		if (m_maxSize.x > 0 && size.x > m_maxSize.x)
		{
			size.x = m_maxSize.x;
		}
		else if (m_minSize.x > 0 && size.x < m_minSize.x)
		{
			size.x = m_minSize.x;
		}

		if (m_maxSize.y > 0 && size.y > m_maxSize.y)
		{
			size.y = m_maxSize.y;
		}
		else if (m_minSize.y > 0 && size.y < m_minSize.y)
		{
			size.y = m_minSize.y;
		}

		return size;
	}

	void Screen::updateScreenRect()
	{
		const Size size = getSize();
		const Size termSize = getTerminalSize();

		int maxScrollPosX = 0;
		int maxScrollPosY = 0;

		if (termSize.x > size.x)
		{
			m_screenRect.first.x = (termSize.x - size.x) / 2;
			m_screenRect.second.x = m_screenRect.first.x + size.x - 1;
		}
		else
		{
			m_screenRect.first.x = 0;
			m_screenRect.second.x = termSize.x - 1;
			maxScrollPosX = size.x - termSize.x;
		}

		if (termSize.y > size.y)
		{
			m_screenRect.first.y = (termSize.y - size.y) / 2;
			m_screenRect.second.y = m_screenRect.first.y + size.y - 1;
		}
		else
		{
			m_screenRect.first.y = 0;
			m_screenRect.second.y = termSize.y - 1;
			maxScrollPosY = size.y - termSize.y;
		}

		if (m_screenRectScrollPos.x > maxScrollPosX)
		{
			m_screenRectScrollPos.x = maxScrollPosX;
		}

		if (m_screenRectScrollPos.y > maxScrollPosY)
		{
			m_screenRectScrollPos.y = maxScrollPosY;
		}
	}

	bool Screen::scrollScreenRectX(int amount)
	{
		if (amount < 0)
		{
			amount = -amount;
			const int maxAmount = m_screenRectScrollPos.x;

			if (amount > maxAmount)
			{
				amount = maxAmount;
			}

			if (amount > 0)
			{
				m_screenRectScrollPos.x -= amount;

				return true;
			}
		}
		else if (amount > 0)
		{
			const int width = getWidth();
			const int termWidth = getTerminalWidth();
			const int maxAmount = (width <= termWidth) ? 0 : width - termWidth - m_screenRectScrollPos.x;

			if (amount > maxAmount)
			{
				amount = maxAmount;
			}

			if (amount > 0)
			{
				m_screenRectScrollPos.x += amount;

				return true;
			}
		}

		return false;
	}

	bool Screen::scrollScreenRectY(int amount)
	{
		if (amount < 0)
		{
			amount = -amount;
			const int maxAmount = m_screenRectScrollPos.y;

			if (amount > maxAmount)
			{
				amount = maxAmount;
			}

			if (amount > 0)
			{
				m_screenRectScrollPos.y -= amount;

				return true;
			}
		}
		else if (amount > 0)
		{
			const int height = getHeight();
			const int termHeight = getTerminalHeight();
			const int maxAmount = (height <= termHeight) ? 0 : height - termHeight - m_screenRectScrollPos.y;

			if (amount > maxAmount)
			{
				amount = maxAmount;
			}

			if (amount > 0)
			{
				m_screenRectScrollPos.y += amount;

				return true;
			}
		}

		return false;
	}
}
