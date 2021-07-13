/**
 * @file
 * @brief Implementation of DialogConnectionColumns class.
 */

#include <string>

#include "DialogConnectionColumns.hpp"
#include "ScreenConnectionList.hpp"
#include "ColorSystem.hpp"

namespace ctp
{
	unsigned int ConnectionColumn::GetDefaultWidth(EType column)
	{
		switch (column)
		{
			case PROTO:            return  6;  // "tcp6"
			case PROTO_STATE:      return 12;  // "ESTABLISHED"

			case SRC_ADDRESS:      return 40;  // IP address or hostname
			case SRC_PORT:         return  6;  // 16-bit unsigned integer or service name
			case SRC_COUNTRY:      return  2;  // two-letter country code
			case SRC_ASN:          return 12;  // "AS" + 32-bit unsigned integer
			case SRC_ASN_ORG_NAME: return 20;  // organization name

			case DST_ADDRESS:      return 40;  // IP address or hostname
			case DST_PORT:         return  6;  // 16-bit unsigned integer or service name
			case DST_COUNTRY:      return  2;  // two-letter country code
			case DST_ASN:          return 12;  // "AS" + 32-bit unsigned integer
			case DST_ASN_ORG_NAME: return 20;  // organization name

			case RX_PACKETS:       return  6;  // 64-bit unsigned integer
			case RX_BYTES:         return  9;  // "1999.9 MB"
			case RX_SPEED:         return 11;  // "1999.9 MB/s"
			case TX_PACKETS:       return  6;  // 64-bit unsigned integer
			case TX_BYTES:         return  9;  // "1999.9 MB"
			case TX_SPEED:         return 11;  // "1999.9 MB/s"
		}

		return 0;
	}

	KString ConnectionColumn::GetLabel(EType column)
	{
		switch (column)
		{
			case PROTO:
			{
				return "PROTO";
			}
			case PROTO_STATE:
			{
				return "STATE";
			}
			case SRC_ADDRESS:
			{
				return "SOURCE";
			}
			case DST_ADDRESS:
			{
				return "DESTINATION";
			}
			case RX_PACKETS:
			case RX_BYTES:
			case RX_SPEED:
			{
				return "DOWN";
			}
			case TX_PACKETS:
			case TX_BYTES:
			case TX_SPEED:
			{
				return "UP";
			}
			case SRC_PORT:
			case SRC_COUNTRY:
			case SRC_ASN:
			case SRC_ASN_ORG_NAME:
			case DST_PORT:
			case DST_COUNTRY:
			case DST_ASN:
			case DST_ASN_ORG_NAME:
			{
				break;
			}
		}

		return KString();
	}

	KString ConnectionColumn::GetName(EType column)
	{
		switch (column)
		{
			case PROTO:            return "Connection protocol";
			case PROTO_STATE:      return "Connection protocol state";

			case SRC_ADDRESS:      return "Source address";
			case SRC_PORT:         return "Source port";
			case SRC_COUNTRY:      return "Source country";
			case SRC_ASN:          return "Source AS number";
			case SRC_ASN_ORG_NAME: return "Source AS organization name";

			case DST_ADDRESS:      return "Destination address";
			case DST_PORT:         return "Destination port";
			case DST_COUNTRY:      return "Destination country";
			case DST_ASN:          return "Destination AS number";
			case DST_ASN_ORG_NAME: return "Destination AS organization name";

			case RX_PACKETS:       return "Number of received packets";
			case RX_BYTES:         return "Number of received bytes";
			case RX_SPEED:         return "Current receive speed";
			case TX_PACKETS:       return "Number of sent packets";
			case TX_BYTES:         return "Number of sent bytes";
			case TX_SPEED:         return "Current receive speed";
		}

		return "?";
	}

	DialogConnectionColumns::DialogConnectionColumns(ScreenConnectionList *parent)
	: Screen({ 50, ConnectionColumn::COLUMN_COUNT + 2 }, { 50, ConnectionColumn::COLUMN_COUNT + 2 }, parent),
	  m_columns(),
	  m_columnConfig{{  // default column configuration
	    { ConnectionColumn::PROTO,            true  },
	    { ConnectionColumn::PROTO_STATE,      false },
	    { ConnectionColumn::SRC_ADDRESS,      true  },
	    { ConnectionColumn::SRC_PORT,         true  },
	    { ConnectionColumn::SRC_COUNTRY,      true  },
	    { ConnectionColumn::SRC_ASN,          false },
	    { ConnectionColumn::SRC_ASN_ORG_NAME, false },
	    { ConnectionColumn::DST_ADDRESS,      true  },
	    { ConnectionColumn::DST_PORT,         true  },
	    { ConnectionColumn::DST_COUNTRY,      true  },
	    { ConnectionColumn::DST_ASN,          false },
	    { ConnectionColumn::DST_ASN_ORG_NAME, false },
	    { ConnectionColumn::RX_PACKETS,       false },
	    { ConnectionColumn::RX_BYTES,         false },
	    { ConnectionColumn::RX_SPEED,         true  },
	    { ConnectionColumn::TX_PACKETS,       false },
	    { ConnectionColumn::TX_BYTES,         false },
	    { ConnectionColumn::TX_SPEED,         true  }
	  }},
	  m_columnConfigOld(m_columnConfig),
	  m_cursorPos(0)
	{
		applyConfig();

		draw();
	}

	void DialogConnectionColumns::open()
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>(getParentScreen());

		parent->pushDialog(this);
	}

	void DialogConnectionColumns::close(bool apply)
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>(getParentScreen());

		parent->removeDialog(this);

		if (apply)
		{
			applyConfig();
			parent->updateColumns();
		}
		else
		{
			restoreConfig();
		}
	}

	void DialogConnectionColumns::applyConfig()
	{
		m_columns.clear();

		bool hasLabelDown = false;
		bool hasLabelUp = false;

		for (ColumnConfig & config : m_columnConfig)
		{
			if (!config.isEnabled())
			{
				continue;
			}

			bool enableLabel = true;

			switch (config.getType())
			{
				case ConnectionColumn::RX_PACKETS:
				case ConnectionColumn::RX_BYTES:
				case ConnectionColumn::RX_SPEED:
				{
					if (!hasLabelDown)
					{
						hasLabelDown = true;
					}
					else
					{
						enableLabel = false;
					}
					break;
				}
				case ConnectionColumn::TX_PACKETS:
				case ConnectionColumn::TX_BYTES:
				case ConnectionColumn::TX_SPEED:
				{
					if (!hasLabelUp)
					{
						hasLabelUp = true;
					}
					else
					{
						enableLabel = false;
					}
					break;
				}
				default:
				{
					break;
				}
			}

			m_columns.emplace_back(config.getType(), config.getWidth(), enableLabel);
		}

		m_columnConfigOld = m_columnConfig;
	}

	void DialogConnectionColumns::restoreConfig()
	{
		m_columnConfig = m_columnConfigOld;

		// restore dialog content
		draw();
	}

	void DialogConnectionColumns::draw()
	{
		box(getWindow(), 0, 0);

		for (unsigned int i = 0; i < m_columnConfig.size(); i++)
		{
			drawEntry(i);
		}
	}

	void DialogConnectionColumns::drawEntry(unsigned int index)
	{
		const ColumnConfig & config = m_columnConfig[index];
		const int cursorAttr = gColorSystem->getAttr(ColorSystem::ATTR_SELECTED_ROW);
		const bool isCursor = (m_cursorPos == index);

		setPos(1, index+1);

		if (isCursor)
		{
			enableAttr(cursorAttr);
		}

		writeString(config.isEnabled() ? " [x] - " : " [ ] - ");
		writeString(config.getName());
		writeString(" - ");
		writeString(std::to_string(config.getWidth()));

		fillEmpty();

		if (isCursor)
		{
			disableAttr(cursorAttr);
		}
	}

	void DialogConnectionColumns::fillEmpty(int count)
	{
		while (count > 0)
		{
			writeChar(' ');
			count--;
		}
	}

	void DialogConnectionColumns::fillEmpty()
	{
		fillEmpty(getWidth() - getPos().x - 1);  // window box
	}

	void DialogConnectionColumns::handleResize()
	{
		draw();
	}

	bool DialogConnectionColumns::handleKey(int ch)
	{
		switch (ch)
		{
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
				const unsigned int maxOffset = m_columnConfig.size() - m_cursorPos - 1;

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
			case KEY_LEFT:
			{
				ColumnConfig & config = m_columnConfig[m_cursorPos];

				if (config.getWidth() > 2)
				{
					config.setWidth(config.getWidth() - 1);
					drawEntry(m_cursorPos);
				}

				return true;
			}
			case KEY_RIGHT:
			{
				ColumnConfig & config = m_columnConfig[m_cursorPos];

				if (config.getWidth() < 99)
				{
					config.setWidth(config.getWidth() + 1);
					drawEntry(m_cursorPos);
				}

				return true;
			}
			case ' ':  // spacebar
			{
				ColumnConfig & config = m_columnConfig[m_cursorPos];

				config.setEnabled(!config.isEnabled());
				drawEntry(m_cursorPos);

				return true;
			}
			case '\015':  // ENTER key
			case '\033':  // ESC key
			{
				const bool apply = (ch == '\015');

				close(apply);

				return true;
			}
		}

		return false;
	}
}
