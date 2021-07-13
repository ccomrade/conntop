/**
 * @file
 * @brief Implementation of ScreenConnectionList class.
 */

#include "ScreenConnectionList.hpp"
#include "ColorSystem.hpp"
#include "App.hpp"
#include "ConnectionList.hpp"
#include "ICollector.hpp"
#include "Client.hpp"
#include "Util.hpp"

namespace ctp
{
	static std::string CreateServerInfoString(const ClientSession & session)
	{
		std::string info = "Connected to server ";

		info += session.getServerName();

		if (!session.getServerEndpoint().isEmpty())
		{
			info += " at ";
			info += session.getServerEndpoint().toString();

			if (session.getServerHostString() != session.getServerEndpoint().getAddress().toString())
			{
				info += " (";
				info += session.getServerHostString();
				info += ")";
			}
		}

		return info;
	}

	static std::string CreateDisconnectInfoString(const ClientSession & session)
	{
		std::string info = "Disconnected";

		if (session.getDisconnectReason() != EDisconnectReason::UNKNOWN)
		{
			info += " (";
			info += session.getDisconnectReasonName();

			if (session.hasDisconnectError())
			{
				info += ": ";
				info += session.getDisconnectErrorString();
			}

			info += ")";
		}

		return info;
	}

	ScreenConnectionList::ScreenConnectionList()
	: Screen({ MINIMUM_WIDTH, MINIMUM_HEIGHT }, { -1, -1 }),
	  m_keyBar(this),
	  m_dialogDetails(this),
	  m_dialogColumns(this),
	  m_dialogSortMode(this),
	  m_columnsTotalWidth(0),
	  m_currentSize(0),
	  m_connectionCount(0),
	  m_scrollOffset(0),
	  m_resolvedPercentage(-1),  // "N/A"
	  m_showCursor(true),
	  m_showHostname(true),
	  m_showPortname(true),
	  m_pausedState(false),
	  m_syncState(false)
	{
		m_keyBar.setCustomKey(FunctionKeyBar::F2, "Column");
		m_keyBar.setCustomKey(FunctionKeyBar::F3, "Sort");
		m_keyBar.setCustomKey(FunctionKeyBar::F4, "Cursor");
		m_keyBar.setCustomKey(FunctionKeyBar::F5, "Host");
		m_keyBar.setCustomKey(FunctionKeyBar::F6, "Port");
		m_keyBar.setCustomKey(FunctionKeyBar::F9, "Pause");

		m_columnsTotalWidth = m_dialogColumns.getColumnsTotalWidth();

		setMinWidth();

		if ((gApp->isClient() && gApp->getClient()->isPaused())
		  || (gApp->hasCollector() && gApp->getCollector()->isPaused()))
		{
			m_pausedState = true;
		}

		// draw screen content
		handleResize();
	}

	void ScreenConnectionList::updateList()
	{
		ConnectionList *pConnectionList = gApp->getConnectionList();

		ConnectionListUpdate update = pConnectionList->getNextUpdate();
		while (!update.isEmpty())
		{
			drawListEntry(update);

			update = pConnectionList->getNextUpdate();
		}

		const unsigned int listSize = pConnectionList->getSize();
		if (m_currentSize != listSize)
		{
			if (m_currentSize > listSize && listSize < getListSize())
			{
				const int posY = getListBeginPos() + listSize;
				const int diff = m_currentSize - listSize;
				for (int i = 0; i < diff; i++)
				{
					setPos(0, posY + i);
					fillEmpty();
				}
			}

			m_currentSize = listSize;
		}

		const unsigned int connectionCount = pConnectionList->getConnectionCount();
		if (m_connectionCount != connectionCount)
		{
			m_connectionCount = connectionCount;
			drawConnectionCount();
		}

		const unsigned int scrollOffset = pConnectionList->getScrollOffset();
		if (m_scrollOffset != scrollOffset)
		{
			m_scrollOffset = scrollOffset;
			drawScrollOffset();
		}

		const int resolvedPercentage = pConnectionList->getResolvedPercentage();
		if (m_resolvedPercentage != resolvedPercentage)
		{
			m_resolvedPercentage = resolvedPercentage;
			drawResolvedPercentage();
		}

		if (pConnectionList->isConnectionDetailRefreshRequired())
		{
			if (hasDialog(&m_dialogDetails))
			{
				const ConnectionData *pData = pConnectionList->getConnectionDetail();
				const int updateFlags = pConnectionList->getConnectionDetailUpdateFlags();
				m_dialogDetails.update(pData, updateFlags);
			}

			pConnectionList->resetConnectionDetailRefresh();
		}
	}

	void ScreenConnectionList::updateColumns()
	{
		m_columnsTotalWidth = m_dialogColumns.getColumnsTotalWidth();

		setMinWidth();

		drawListHeader();

		gApp->getConnectionList()->invalidateAllVisible();
		updateList();
	}

	void ScreenConnectionList::updateClientInfo(const ClientEvent & event)
	{
		if (gApp->isClient())
		{
			switch (event.getType())
			{
				case ClientEvent::DISCONNECTED:
				case ClientEvent::CONNECT_STARTED:
				case ClientEvent::DISCONNECT_STARTED:
				case ClientEvent::CONNECTION_ESTABLISHED:
				case ClientEvent::SESSION_ESTABLISHED:
				{
					drawCollectorInfo();
					break;
				}
				case ClientEvent::SYNC_STATE_CHANGED:
				{
					m_syncState = gApp->getClient()->isSynchronized();
					drawSyncState();
					break;
				}
				case ClientEvent::SERVER_UPDATE_TICK:
				case ClientEvent::NEW_DATA_AVAILABLE:
				{
					break;
				}
			}
		}
	}

	void ScreenConnectionList::updateSortMode()
	{
		gApp->getConnectionList()->setSortMode(m_dialogSortMode.getSortMode(), m_dialogSortMode.isSortAscending());

		updateList();
	}

	void ScreenConnectionList::drawStatic()
	{
		const int labelAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_LABEL);

		m_keyBar.draw();

		setPos(1, 1);
		enableAttr(labelAttr);
		writeString("Connections:          | Resolved:       | Scroll:        | ");
		disableAttr(labelAttr);

		if (gApp->hasCollector())
		{
			setPos(1, 2);
			enableAttr(labelAttr);
			writeString("Collector: ");
			disableAttr(labelAttr);
		}
		else if (gApp->isClient())
		{
			setPos(1, 2);
			enableAttr(labelAttr);
			writeString("Client: ");
			disableAttr(labelAttr);
		}
	}

	void ScreenConnectionList::drawListHeader()
	{
		const int listHeaderAttr = gColorSystem->getAttr(ColorSystem::ATTR_LIST_HEADER);

		setPos (0, STATUS_HEADER_HEIGHT);
		enableAttr(listHeaderAttr);

		unsigned int remainingSpace = getWidth();
		for (const ConnectionColumn & column : m_dialogColumns.getColumns())
		{
			unsigned int space = (remainingSpace > column.getWidth()) ? column.getWidth() : remainingSpace;

			KString label = column.getLabel();
			if (label.empty())
			{
				fillEmpty(space);
			}
			else
			{
				writeString(label, space);
				fillEmpty(space - label.length());
			}

			remainingSpace -= space;

			if (remainingSpace > 0)
			{
				// column separator
				writeChar(' ');
				remainingSpace--;
			}
		}

		if (remainingSpace > 0)
		{
			fillEmpty();
		}

		disableAttr(listHeaderAttr);
	}

	void ScreenConnectionList::drawConnectionCount()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		setPos(14, 1);
		enableAttr(valueAttr);
		writef("%-8u", m_connectionCount);
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawResolvedPercentage()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		setPos(35, 1);
		enableAttr(valueAttr);

		if (m_resolvedPercentage < 0)
		{
			writeString("N/A");
		}
		else if (m_resolvedPercentage > 0 && m_resolvedPercentage < 1000)
		{
			double percentageFloat = m_resolvedPercentage;
			percentageFloat /= 10;
			writef("%.1f%%", percentageFloat);
		}
		else
		{
			writeString(std::to_string(m_resolvedPercentage / 10));
			writeChar('%');
		}

		fillEmpty(40 - getPos().x);

		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawScrollOffset()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		setPos(51, 1);
		enableAttr(valueAttr);
		writef("%-6u", m_scrollOffset);
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawHostnameState()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		setPos(60, 1);
		enableAttr(valueAttr);
		writeString((m_showHostname) ? "HOST" : "    ");
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawPortnameState()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		setPos(65, 1);
		enableAttr(valueAttr);
		writeString((m_showPortname) ? "PORT" : "    ");
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawPausedState()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_IMPORTANT_RED);

		setPos(70, 1);
		enableAttr(valueAttr);
		writeString((m_pausedState) ? "PAUSE" : "     ");
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawSyncState()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_IMPORTANT_GREEN);

		setPos(76, 1);
		enableAttr(valueAttr);
		writeString((m_syncState) ? "SYNC" : "    ");
		disableAttr(valueAttr);
	}

	void ScreenConnectionList::drawCollectorInfo()
	{
		const int valueAttr = gColorSystem->getAttr(ColorSystem::ATTR_STATUS_VALUE);

		if (gApp->isClient())
		{
			setPos(9, 2);

			const unsigned int space = getWidth() - getPos().x;
			const Client *pClient = gApp->getClient();

			if (pClient->isConnected())
			{
				drawDataLeft(CreateServerInfoString(pClient->getSession()), space, valueAttr);
			}
			else if (pClient->isConnecting())
			{
				drawDataLeft("Connecting...", space, valueAttr);
			}
			else if (pClient->isDisconnecting())
			{
				drawDataLeft("Disconnecting...", space, valueAttr);
			}
			else  // client is disconnected
			{
				drawDataLeft(CreateDisconnectInfoString(pClient->getSession()), space, valueAttr);
			}

			const bool syncState = pClient->isSynchronized();

			if (m_syncState != syncState)
			{
				m_syncState = syncState;
				drawSyncState();
			}
		}
		else if (gApp->hasCollector())
		{
			setPos(12, 2);

			const unsigned int space = getWidth() - getPos().x;

			drawDataLeft(gApp->getCollector()->getName(), space, valueAttr);
		}
	}

	void ScreenConnectionList::drawListEntry(const ConnectionListUpdate & update)
	{
		const ConnectionData *pData = update.getData();
		const int updateFlags = update.getUpdateFlags();
		const bool isCursor = update.isCursor() && m_showCursor;

		const int cursorAttr = gColorSystem->getAttr(ColorSystem::ATTR_SELECTED_ROW);
		const int defaultAttr = (isCursor) ? cursorAttr : gColorSystem->getAttr(ColorSystem::ATTR_DEFAULT);

		const int unresolvedAttr = gColorSystem->getAttr((isCursor) ?
		  ColorSystem::ATTR_UNRESOLVED_SELECTED :
		  ColorSystem::ATTR_UNRESOLVED
		);

		const int trafficAttr = gColorSystem->getAttr((isCursor) ?
		  ColorSystem::ATTR_TRAFFIC_SELECTED :
		  ColorSystem::ATTR_TRAFFIC
		);

		int posX = 0;
		int posY = getListBeginPos() + update.getRowNumber();

		setPos(posX, posY);

		const std::vector<ConnectionColumn> & columnList = m_dialogColumns.getColumns();

		for (unsigned int i = 0; i < columnList.size(); i++)
		{
			const ConnectionColumn & column = columnList[i];

			switch (column.getType())
			{
				case ConnectionColumn::PROTO:
				{
					if (!(updateFlags & EConnectionUpdateFlags::PROTO))
					{
						break;
					}

					int attr = defaultAttr;

					switch (pData->getType())
					{
						case EConnectionType::UDP4:
						case EConnectionType::UDP6:
						{
							attr = gColorSystem->getAttr((isCursor) ?
							  ColorSystem::ATTR_PROTO_UDP_SELECTED :
							  ColorSystem::ATTR_PROTO_UDP
							);
							break;
						}
						case EConnectionType::TCP4:
						case EConnectionType::TCP6:
						{
							attr = gColorSystem->getAttr((isCursor) ?
							  ColorSystem::ATTR_PROTO_TCP_SELECTED :
							  ColorSystem::ATTR_PROTO_TCP
							);
							break;
						}
					}

					drawDataLeft(pData->getTypeName(), column.getWidth(), attr);

					break;
				}
				case ConnectionColumn::PROTO_STATE:
				{
					if (!(updateFlags & EConnectionUpdateFlags::PROTO_STATE))
					{
						break;
					}

					drawDataLeft(pData->getStateName(), column.getWidth(), defaultAttr);

					break;
				}
				case ConnectionColumn::SRC_ADDRESS:
				{
					if (!(updateFlags & EConnectionUpdateFlags::SRC_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getSrcAddr();
					const int attr = (address.isHostnameResolved()) ? defaultAttr : unresolvedAttr;

					if (address.isHostnameAvailable() && m_showHostname)
					{
						drawDataRight(address.getHostnameString(), column.getWidth(), attr);
					}
					else
					{
						drawDataRight(address.getNumericString(), column.getWidth(), attr);
					}

					break;
				}
				case ConnectionColumn::DST_ADDRESS:
				{
					if (!(updateFlags & EConnectionUpdateFlags::DST_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getDstAddr();
					const int attr = (address.isHostnameResolved()) ? defaultAttr : unresolvedAttr;

					if (address.isHostnameAvailable() && m_showHostname)
					{
						drawDataRight(address.getHostnameString(), column.getWidth(), attr);
					}
					else
					{
						drawDataRight(address.getNumericString(), column.getWidth(), attr);
					}

					break;
				}
				case ConnectionColumn::SRC_PORT:
				{
					if (!(updateFlags & EConnectionUpdateFlags::SRC_PORT))
					{
						break;
					}

					if (pData->hasPorts())
					{
						const PortData & port = pData->getSrcPort();
						const int attr = (port.isServiceResolved()) ? defaultAttr : unresolvedAttr;

						if (port.isServiceAvailable() && m_showPortname)
						{
							drawDataLeft(port.getServiceString(), column.getWidth(), attr);
						}
						else
						{
							drawDataLeft(port.getNumericString(), column.getWidth(), attr);
						}
					}
					else
					{
						drawDataEmpty(column.getWidth(), defaultAttr);
					}

					break;
				}
				case ConnectionColumn::DST_PORT:
				{
					if (!(updateFlags & EConnectionUpdateFlags::DST_PORT))
					{
						break;
					}

					if (pData->hasPorts())
					{
						const PortData & port = pData->getDstPort();
						const int attr = (port.isServiceResolved()) ? defaultAttr : unresolvedAttr;

						if (port.isServiceAvailable() && m_showPortname)
						{
							drawDataLeft(port.getServiceString(), column.getWidth(), attr);
						}
						else
						{
							drawDataLeft(port.getNumericString(), column.getWidth(), attr);
						}
					}
					else
					{
						drawDataEmpty(column.getWidth(), defaultAttr);
					}

					break;
				}
				case ConnectionColumn::SRC_COUNTRY:
				{
					if (!(updateFlags & EConnectionUpdateFlags::SRC_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getSrcAddr();
					const int attr = gColorSystem->getAttr((isCursor) ?
					  ColorSystem::ATTR_COUNTRY_CODE_SELECTED :
					  ColorSystem::ATTR_COUNTRY_CODE
					);

					if (address.isCountryAvailable())
					{
						drawDataLeft(address.getCountry().getCodeString(), column.getWidth(), attr);
					}
					else
					{
						drawDataEmpty(column.getWidth(), attr);
					}

					break;
				}
				case ConnectionColumn::DST_COUNTRY:
				{
					if (!(updateFlags & EConnectionUpdateFlags::DST_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getDstAddr();
					const int attr = gColorSystem->getAttr((isCursor) ?
					  ColorSystem::ATTR_COUNTRY_CODE_SELECTED :
					  ColorSystem::ATTR_COUNTRY_CODE
					);

					if (address.isCountryAvailable())
					{
						drawDataLeft(address.getCountry().getCodeString(), column.getWidth(), attr);
					}
					else
					{
						drawDataEmpty(column.getWidth(), attr);
					}

					break;
				}
				case ConnectionColumn::SRC_ASN:
				{
					if (!(updateFlags & EConnectionUpdateFlags::SRC_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getSrcAddr();

					drawDataLeft(address.getASN().getString(), column.getWidth(), defaultAttr);

					break;
				}
				case ConnectionColumn::DST_ASN:
				{
					if (!(updateFlags & EConnectionUpdateFlags::DST_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getDstAddr();

					drawDataLeft(address.getASN().getString(), column.getWidth(), defaultAttr);

					break;
				}
				case ConnectionColumn::SRC_ASN_ORG_NAME:
				{
					if (!(updateFlags & EConnectionUpdateFlags::SRC_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getSrcAddr();

					drawDataLeft(address.getASN().getOrgName(), column.getWidth(), defaultAttr);

					break;
				}
				case ConnectionColumn::DST_ASN_ORG_NAME:
				{
					if (!(updateFlags & EConnectionUpdateFlags::DST_ADDRESS))
					{
						break;
					}

					const AddressData & address = pData->getDstAddr();

					drawDataLeft(address.getASN().getOrgName(), column.getWidth(), defaultAttr);

					break;
				}
				case ConnectionColumn::RX_PACKETS:
				{
					if (!(updateFlags & EConnectionUpdateFlags::RX_PACKETS))
					{
						break;
					}

					const std::string valueString = std::to_string(pData->getRXPackets());

					drawDataRight(valueString, column.getWidth(), trafficAttr);

					break;
				}
				case ConnectionColumn::TX_PACKETS:
				{
					if (!(updateFlags & EConnectionUpdateFlags::TX_PACKETS))
					{
						break;
					}

					const std::string valueString = std::to_string(pData->getTXPackets());

					drawDataRight(valueString, column.getWidth(), trafficAttr);

					break;
				}
				case ConnectionColumn::RX_BYTES:
				{
					if (!(updateFlags & EConnectionUpdateFlags::RX_BYTES))
					{
						break;
					}

					const std::string valueString = Util::GetHumanReadableSize(pData->getRXBytes());

					drawDataRight(valueString, column.getWidth(), trafficAttr);

					break;
				}
				case ConnectionColumn::TX_BYTES:
				{
					if (!(updateFlags & EConnectionUpdateFlags::TX_BYTES))
					{
						break;
					}

					const std::string valueString = Util::GetHumanReadableSize(pData->getTXBytes());

					drawDataRight(valueString, column.getWidth(), trafficAttr);

					break;
				}
				case ConnectionColumn::RX_SPEED:
				{
					if (!(updateFlags & EConnectionUpdateFlags::RX_SPEED))
					{
						break;
					}

					const uint64_t value = pData->getRXSpeed();

					if (value > 0)
					{
						std::string valueString = Util::GetHumanReadableSize(value);
						valueString += "/s";
						drawDataRight(valueString, column.getWidth(), trafficAttr);
					}
					else
					{
						drawDataEmpty(column.getWidth(), trafficAttr);
					}

					break;
				}
				case ConnectionColumn::TX_SPEED:
				{
					if (!(updateFlags & EConnectionUpdateFlags::TX_SPEED))
					{
						break;
					}

					const uint64_t value = pData->getTXSpeed();

					if (value > 0)
					{
						std::string valueString = Util::GetHumanReadableSize(value);
						valueString += "/s";
						drawDataRight(valueString, column.getWidth(), trafficAttr);
					}
					else
					{
						drawDataEmpty(column.getWidth(), trafficAttr);
					}

					break;
				}
			}

			posX += column.getWidth();

			setPos(posX, posY);

			if ((i+1) < columnList.size())  // check if current column is not the last column
			{
				if (isCursor)
				{
					enableAttr(cursorAttr);
				}

				// column separator
				writeChar(' ');
				posX++;

				if (isCursor)
				{
					disableAttr(cursorAttr);
				}
			}
		}

		const unsigned int width = getWidth();
		if (width > m_columnsTotalWidth)
		{
			fillEmpty();
		}
	}

	void ScreenConnectionList::drawDataLeft(const KString & data, unsigned int width, int attr)
	{
		enableAttr(attr);

		if (data.length() <= width)
		{
			writeString(data);
			fillEmpty(width - data.length());
		}
		else
		{
			if (width > 2)
			{
				writeString(data, width-2);
			}

			writeString("..");
		}

		disableAttr(attr);
	}

	void ScreenConnectionList::drawDataRight(const KString & data, unsigned int width, int attr)
	{
		enableAttr(attr);

		if (data.length() <= width)
		{
			fillEmpty(width - data.length());
			writeString(data);
		}
		else
		{
			if (width > 2)
			{
				writeString(data, width-2);
			}

			writeString("..");
		}

		disableAttr(attr);
	}

	void ScreenConnectionList::drawDataEmpty(unsigned int width, int attr)
	{
		enableAttr(attr);

		fillEmpty(width);

		disableAttr(attr);
	}

	void ScreenConnectionList::clearList()
	{
		const int basePosY = getListBeginPos();
		const int listSize = getListSize();
		const int width = getWidth();

		for (int i = 0; i < listSize; i++)
		{
			setPos(0, basePosY + i);
			fillEmpty(width);
		}
	}

	void ScreenConnectionList::fillEmpty(int count)
	{
		while (count > 0)
		{
			writeChar(' ');
			count--;
		}
	}

	void ScreenConnectionList::fillEmpty()
	{
		fillEmpty(getWidth() - getPos().x);
	}

	void ScreenConnectionList::setMinWidth()
	{
		unsigned int minWidth = getMinSize().x;

		if (minWidth != m_columnsTotalWidth)
		{
			minWidth = m_columnsTotalWidth;

			if (minWidth < MINIMUM_WIDTH)
			{
				minWidth = MINIMUM_WIDTH;
			}

			Size minSize = getMinSize();
			minSize.x = minWidth;

			setMinSize(minSize);
		}
	}

	void ScreenConnectionList::handleResize()
	{
		drawStatic();
		drawListHeader();
		drawConnectionCount();
		drawResolvedPercentage();
		drawScrollOffset();
		drawHostnameState();
		drawPortnameState();
		drawPausedState();
		drawSyncState();
		drawCollectorInfo();

		gApp->getConnectionList()->setTargetSize(getListSize());
		gApp->getConnectionList()->invalidateAllVisible();

		updateList();
	}

	bool ScreenConnectionList::handleKey(int ch)
	{
		switch (ch)
		{
			case KEY_UP:
			case KEY_PPAGE:
			{
				if (getListSize() > 0)
				{
					const int offset = (ch == KEY_PPAGE) ? getListSize()-1 : 1;

					if (m_showCursor)
					{
						gApp->getConnectionList()->moveCursor(-offset);
					}
					else
					{
						gApp->getConnectionList()->doScroll(-offset);
					}

					if (gApp->getConnectionList()->isRefreshRequired())
					{
						updateList();
					}
				}

				return true;
			}
			case KEY_DOWN:
			case KEY_NPAGE:
			{
				if (getListSize() > 0)
				{
					const int offset = (ch == KEY_NPAGE) ? getListSize()-1 : 1;

					if (m_showCursor)
					{
						gApp->getConnectionList()->moveCursor(offset);
					}
					else
					{
						gApp->getConnectionList()->doScroll(offset);
					}

					if (gApp->getConnectionList()->isRefreshRequired())
					{
						updateList();
					}
				}

				return true;
			}
			case '\015':  // ENTER key
			{
				if (m_showCursor)
				{
					gApp->getConnectionList()->initConnectionDetail();

					if (gApp->getConnectionList()->hasConnectionDetail())
					{
						m_dialogDetails.open(gApp->getConnectionList()->getConnectionDetail());
					}
				}

				return true;
			}
			case KEY_F(2):
			{
				if (hasDialog(&m_dialogColumns))
				{
					m_dialogColumns.close();
				}
				else
				{
					m_dialogColumns.open();
				}

				return true;
			}
			case KEY_F(3):
			{
				if (hasDialog(&m_dialogSortMode))
				{
					m_dialogSortMode.close();
				}
				else
				{
					m_dialogSortMode.open();
				}

				return true;
			}
			case KEY_F(4):
			{
				m_showCursor = !m_showCursor;

				gApp->getConnectionList()->invalidateCursor();
				updateList();

				return true;
			}
			case KEY_F(5):
			{
				m_showHostname = !m_showHostname;
				drawHostnameState();

				gApp->getConnectionList()->invalidateAllVisible();
				updateList();

				return true;
			}
			case KEY_F(6):
			{
				m_showPortname = !m_showPortname;
				drawPortnameState();

				gApp->getConnectionList()->invalidateAllVisible();
				updateList();

				return true;
			}
			case ' ':  // spacebar
			case KEY_F(9):
			{
				if (gApp->hasCollector())
				{
					m_pausedState = !m_pausedState;
					drawPausedState();

					gApp->getCollector()->setPaused(m_pausedState);
				}
				else if (gApp->isClient())
				{
					m_pausedState = !m_pausedState;
					drawPausedState();

					gApp->getClient()->setPaused(m_pausedState);
				}

				return true;
			}
		}

		return m_keyBar.handlePressedKey(ch);
	}
}
