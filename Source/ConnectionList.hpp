/**
 * @file
 * @brief ConnectionList class.
 */

#pragma once

#include <deque>

#include "ConnectionStorage.hpp"
#include "Resolver.hpp"

namespace ctp
{
	class ConnectionListUpdate
	{
		const ConnectionData *m_pData;
		int m_updateFlags;
		unsigned int m_rowNumber;
		bool m_isCursor;

	public:
		ConnectionListUpdate()
		: m_pData(nullptr),
		  m_updateFlags(-1),
		  m_rowNumber(0),
		  m_isCursor(false)
		{
		}

		ConnectionListUpdate(const ConnectionData *pData, int updateFlags, unsigned int rowNumber, bool isCursor)
		: m_pData(pData),
		  m_updateFlags(updateFlags),
		  m_rowNumber(rowNumber),
		  m_isCursor(isCursor)
		{
		}

		bool isEmpty() const
		{
			return m_pData == nullptr;
		}

		const ConnectionData *getData() const
		{
			return m_pData;
		}

		int getUpdateFlags() const
		{
			return m_updateFlags;
		}

		unsigned int getRowNumber() const
		{
			return m_rowNumber;
		}

		bool isCursor() const
		{
			return m_isCursor;
		}
	};

	class ConnectionList final : public IConnectionUpdateCallback
	{
		struct Item
		{
			const ConnectionData *pConnection;
			int updateFlags;
			bool isRefreshRequired;

			Item(const ConnectionData *pData, int dataUpdateFlags = -1)
			: pConnection(pData),
			  updateFlags(dataUpdateFlags),
			  isRefreshRequired(true)
			{
			}
		};

		using CompareFunction = bool (*)(const Item & a, const Item & b);  // operator<

		ConnectionStorage m_storage;
		std::deque<Item> m_list;
		unsigned int m_targetSize;
		unsigned int m_scrollOffset;
		unsigned int m_cursorPos;
		unsigned long m_dataTotalCount;
		unsigned long m_dataResolvedCount;
		const ConnectionData *m_connectionDetail;
		int m_connectionDetailUpdateFlags;
		EConnectionSortMode m_sortMode;
		bool m_isSortAscending;
		bool m_isRefreshRequired;
		CompareFunction m_compareFunc;

		unsigned int getCursorMaxPos() const
		{
			const unsigned int visibleSize = getSize();
			return (visibleSize > 0) ? visibleSize-1 : 0;
		}

		unsigned int getListBeginIndex() const
		{
			return (m_list.size() > m_scrollOffset) ? m_scrollOffset : 0;
		}

		std::deque<Item>::iterator getListBeginIt()
		{
			return (m_list.size() > m_scrollOffset) ? m_list.begin() + m_scrollOffset : m_list.end();
		}

		bool containsItem(const Item & item) const
		{
			for (const Item & listItem : m_list)
			{
				if (listItem.pConnection == item.pConnection)
					return true;
			}
			return false;
		}

		void fill();
		void sort();
		void insertItem(const Item & item);
		void handleNewConnection(const ConnectionData & connection);
		void handleRemovedConnection(void *pConnection);
		void updateCompareFunc();

		static void ResolverCallbackAddress(AddressData & address, ResolvedAddress & resolved, void *param);
		static void ResolverCallbackPort(PortData & port, ResolvedPort & resolved, void *param);

	public:
		ConnectionList();

		// IConnectionUpdateCallback

		AddressData *getAddress(const IAddress & address, bool add = false) override;
		PortData *getPort(const Port & port, bool add = false) override;
		ConnectionData *find(const Connection & connection) override;
		ConnectionData *add(const Connection & connection) override;
		ConnectionData *add(const Connection & connection, const ConnectionTraffic & traffic, int state) override;
		void update(const ConnectionData & data, int updateFlags) override;
		void remove(const Connection & connection) override;
		void clear() override;

		void addressDataUpdated(const AddressData & address);
		void portDataUpdated(const PortData & port);

		ConnectionListUpdate getNextUpdate();
		void invalidateAllVisible();
		void invalidateCursor();
		void moveCursor(int amount);
		void doScroll(int amount);
		void initConnectionDetail();
		void resetConnectionDetailRefresh();
		void setTargetSize(unsigned int size);
		void setSortMode(EConnectionSortMode sortMode, bool isAscending = false);

		bool isRefreshRequired() const
		{
			return m_isRefreshRequired;
		}

		bool isConnectionDetailRefreshRequired() const
		{
			return m_connectionDetailUpdateFlags != 0;
		}

		bool isEverythingResolved() const
		{
			return m_dataResolvedCount == m_dataTotalCount;
		}

		bool hasConnectionDetail() const
		{
			return m_connectionDetail != nullptr;
		}

		unsigned int getSize() const
		{
			return (m_list.size() > m_scrollOffset) ? m_list.size() - m_scrollOffset : 0;
		}

		unsigned int getTargetSize() const
		{
			return m_targetSize;
		}

		unsigned int getConnectionCount() const
		{
			return m_storage.getConnectionCount();
		}

		unsigned int getScrollOffset() const
		{
			return m_scrollOffset;
		}

		unsigned int getCursorPos() const
		{
			return m_cursorPos;
		}

		EConnectionSortMode getSortMode() const
		{
			return m_sortMode;
		}

		int getResolvedPercentage() const
		{
			return (m_dataTotalCount > 0) ? (m_dataResolvedCount * 1000) / m_dataTotalCount : -1;
		}

		int getConnectionDetailUpdateFlags() const
		{
			return m_connectionDetailUpdateFlags;
		}

		const ConnectionData *getConnectionDetail() const
		{
			return m_connectionDetail;
		}
	};
}
