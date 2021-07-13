/**
 * @file
 * @brief Implementation of DialogConnectionSort class.
 */

#include "DialogConnectionSort.hpp"
#include "ScreenConnectionList.hpp"
#include "ColorSystem.hpp"

namespace ctp
{
	const std::array<KString, DialogConnectionSort::SORT_MODE_COUNT> DialogConnectionSort::SORT_MODE_NAMES = {
		"None",
		"Connection protocol",
		"Connection protocol state",
		"Source address",
		"Source hostname",
		"Source ASN",
		"Source country",
		"Source port",
		"Source service",
		"Destination address",
		"Destination hostname",
		"Destination ASN",
		"Destination country",
		"Destination port",
		"Destination service",
		"Received packets",
		"Sent packets",
		"Received bytes",
		"Sent bytes",
		"Receive speed",
		"Send speed"
	};

	DialogConnectionSort::DialogConnectionSort(ScreenConnectionList *parent)
	: Screen({ 40, SORT_MODE_COUNT + 4 }, { 40, SORT_MODE_COUNT + 4 }, parent),
	  m_sortMode(EConnectionSortMode::NONE),
	  m_cursorPos(0),
	  m_isSortAscending(false),
	  m_checkBoxState(false)
	{
		draw();
	}

	void DialogConnectionSort::open()
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>(getParentScreen());

		parent->pushDialog(this);
	}

	void DialogConnectionSort::close(bool apply)
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>(getParentScreen());

		parent->removeDialog(this);

		if (apply)
		{
			applyConfig();
			parent->updateSortMode();
		}
		else
		{
			restoreConfig();
		}
	}

	void DialogConnectionSort::applyConfig()
	{
		m_sortMode = static_cast<EConnectionSortMode>((m_cursorPos < SORT_MODE_COUNT) ? m_cursorPos : 0);
		m_isSortAscending = m_checkBoxState;
	}

	void DialogConnectionSort::restoreConfig()
	{
		m_cursorPos = static_cast<unsigned int>(m_sortMode);
		m_checkBoxState = m_isSortAscending;

		// restore dialog content
		draw();
	}

	void DialogConnectionSort::draw()
	{
		box(getWindow(), 0, 0);

		for (unsigned int i = 0; i < SORT_MODE_COUNT; i++)
		{
			drawEntry(i);
		}

		drawCheckbox();
	}

	void DialogConnectionSort::drawEntry(unsigned int index)
	{
		if (index >= SORT_MODE_COUNT)
		{
			drawCheckbox();

			return;
		}

		const int cursorAttr = gColorSystem->getAttr(ColorSystem::ATTR_SELECTED_ROW);
		const bool isCursor = (m_cursorPos == index);

		if (isCursor)
		{
			enableAttr(cursorAttr);
		}

		setPos(1, index+1);
		writeString(SORT_MODE_NAMES[index]);

		fillEmpty();

		if (isCursor)
		{
			disableAttr(cursorAttr);
		}
	}

	void DialogConnectionSort::drawCheckbox()
	{
		const int cursorAttr = gColorSystem->getAttr(ColorSystem::ATTR_SELECTED_ROW);
		const bool isCursor = (m_cursorPos == SORT_MODE_COUNT);

		if (isCursor)
		{
			enableAttr(cursorAttr);
		}

		setPos(1, SORT_MODE_COUNT+2);
		writeString(m_checkBoxState ? "[x] - " : "[ ] - ");
		writeString("Ascending sort");

		fillEmpty();

		if (isCursor)
		{
			disableAttr(cursorAttr);
		}
	}

	void DialogConnectionSort::fillEmpty(int count)
	{
		while (count > 0)
		{
			writeChar(' ');
			count--;
		}
	}

	void DialogConnectionSort::fillEmpty()
	{
		fillEmpty(getWidth() - getPos().x - 1);  // window box
	}

	void DialogConnectionSort::handleResize()
	{
		draw();
	}

	bool DialogConnectionSort::handleKey(int ch)
	{
		switch (ch)
		{
			case KEY_LEFT:
			case KEY_RIGHT:
			{
				return true;
			}
			case KEY_UP:
			case KEY_PPAGE:
			{
				const unsigned int maxOffset = m_cursorPos;

				unsigned int offset = (ch == KEY_PPAGE && getHeight() > 1) ? getHeight()-1 : 1;
				if (offset > maxOffset)
				{
					offset = maxOffset;
				}

				if (offset > 0)
				{
					m_cursorPos -= offset;
					drawEntry(m_cursorPos + offset);
					drawEntry(m_cursorPos);
				}

				return true;
			}
			case KEY_DOWN:
			case KEY_NPAGE:
			{
				const unsigned int maxOffset = SORT_MODE_COUNT - m_cursorPos;

				unsigned int offset = (ch == KEY_NPAGE && getHeight() > 1) ? getHeight()-1 : 1;
				if (offset > maxOffset)
				{
					offset = maxOffset;
				}

				if (offset > 0)
				{
					m_cursorPos += offset;
					drawEntry(m_cursorPos - offset);
					drawEntry(m_cursorPos);
				}

				return true;
			}
			case ' ':  // spacebar
			{
				if (m_cursorPos == SORT_MODE_COUNT)
				{
					m_checkBoxState = !m_checkBoxState;
					drawCheckbox();
				}

				return true;
			}
			case '\015':  // ENTER key
			{
				if (m_cursorPos < SORT_MODE_COUNT)
				{
					close(true);
				}

				return true;
			}
			case '\033':  // ESC key
			{
				close();

				return true;
			}
		}

		return false;
	}
}
