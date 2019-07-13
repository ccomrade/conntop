/**
 * @file
 * @brief Implementation of ConnectionList class.
 */

#include <algorithm>

#include "ConnectionList.hpp"
#include "App.hpp"
#include "IUI.hpp"

namespace ctp
{
	ConnectionList::ConnectionList()
	: m_storage(),
	  m_list(),
	  m_targetSize(0),
	  m_scrollOffset(0),
	  m_cursorPos(0),
	  m_dataTotalCount(0),
	  m_dataResolvedCount(0),
	  m_connectionDetail(nullptr),
	  m_connectionDetailUpdateFlags(0),
	  m_sortMode(EConnectionSortMode::NONE),
	  m_isSortAscending(false),
	  m_isRefreshRequired(true),
	  m_compareFunc(nullptr)
	{
		updateCompareFunc();
	}

	AddressData *ConnectionList::getAddress( const IAddress & address, bool add )
	{
		bool isNew = false;
		AddressData *pData = nullptr;

		switch ( address.getType() )
		{
			case EAddressType::IP4:
			{
				if ( add )
				{
					auto result = m_storage.addIP4Address( static_cast<const AddressIP4&>( address ) );
					pData = result.first;
					isNew = result.second;
				}
				else
				{
					pData = m_storage.getIP4Address( static_cast<const AddressIP4&>( address ) );
				}
				break;
			}
			case EAddressType::IP6:
			{
				if ( add )
				{
					auto result = m_storage.addIP6Address( static_cast<const AddressIP6&>( address ) );
					pData = result.first;
					isNew = result.second;
				}
				else
				{
					pData = m_storage.getIP6Address( static_cast<const AddressIP6&>( address ) );
				}
				break;
			}
		}

		if ( isNew )
		{
			m_dataTotalCount++;
			gApp->getResolver()->resolveAddress( *pData, ResolverCallbackAddress, this );
		}

		return pData;
	}

	PortData *ConnectionList::getPort( const Port & port, bool add )
	{
		bool isNew = false;
		PortData *pData = nullptr;

		if ( add )
		{
			auto result = m_storage.addPort( port );
			pData = result.first;
			isNew = result.second;
		}
		else
		{
			pData = m_storage.getPort( port );
		}

		if ( isNew )
		{
			m_dataTotalCount++;
			gApp->getResolver()->resolvePort( *pData, ResolverCallbackPort, this );
		}

		return pData;
	}

	ConnectionData *ConnectionList::find( const Connection & connection )
	{
		return m_storage.getConnection( connection );
	}

	ConnectionData *ConnectionList::add( const Connection & connection )
	{
		auto result = m_storage.addConnection( connection );

		ConnectionData *pData = result.first;
		if ( result.second )
		{
			handleNewConnection( *pData );
		}

		return pData;
	}

	ConnectionData *ConnectionList::add( const Connection & connection, const ConnectionTraffic & traffic, int state )
	{
		auto result = m_storage.addConnection( connection, traffic, state );

		ConnectionData *pData = result.first;
		if ( result.second )
		{
			handleNewConnection( *pData );
		}

		return pData;
	}

	void ConnectionList::update( const ConnectionData & data, int updateFlags )
	{
		if ( m_connectionDetail == &data )
		{
			m_connectionDetailUpdateFlags = updateFlags;
		}

		if ( m_list.empty() )
		{
			return;
		}

		unsigned int i = 0;
		for ( ; i < m_list.size(); i++ )
		{
			if ( m_list[i].pConnection == &data )
			{
				// check if the updated connection is in visible part of the list
				if ( i >= getListBeginIndex() )
				{
					m_list[i].isRefreshRequired = true;
					m_list[i].updateFlags |= updateFlags;
					m_isRefreshRequired = true;
				}

				break;
			}
		}

		if ( i < m_list.size() )
		{
			const Item & item = m_list[i];

			bool isSortRequired = false;
			bool isFillRequired = false;

			// check if the updated connection is not the first
			if ( i > 0 )
			{
				// compare with previous connection in the list
				if ( m_compareFunc( item, m_list[i-1] ) )
				{
					isSortRequired = true;
				}
			}

			// check if the updated connection is not the last
			if ( (i+1) < m_list.size() )
			{
				// compare with next connection in the list
				if ( m_compareFunc( m_list[i+1], item ) )
				{
					isSortRequired = true;
				}

				// compare with the last connection in the list
				if ( m_compareFunc( m_list.back(), item ) )
				{
					// maybe the updated connection shouldn't be in the list anymore
					isFillRequired = true;
				}
			}
			else if ( ! isSortRequired )
			{
				// the connection is already at the end of the list, so maybe it shouldn't be there anymore
				isFillRequired = true;
			}

			if ( isSortRequired )
			{
				sort();
			}

			if ( isFillRequired )
			{
				fill();
			}
		}
		else
		{
			Item item( &data );

			// the updated connection is not in the list but maybe it should be there now
			if ( m_compareFunc( item, m_list.back() ) )
			{
				insertItem( item );
			}
		}
	}

	void ConnectionList::remove( const Connection & connection )
	{
		void *pData = m_storage.removeConnection( connection );
		if ( pData )
		{
			handleRemovedConnection( pData );
		}
	}

	void ConnectionList::clear()
	{
		m_list.clear();
		m_storage.clearConnections();

		m_cursorPos = 0;

		m_connectionDetail = nullptr;
		m_connectionDetailUpdateFlags = -1;

		m_isRefreshRequired = true;
	}

	void ConnectionList::addressDataUpdated( const AddressData & address )
	{
		if ( m_connectionDetail )
		{
			if ( &m_connectionDetail->getSrcAddr() == &address )
			{
				m_connectionDetailUpdateFlags |= EConnectionUpdateFlags::SRC_ADDRESS;
			}

			if ( &m_connectionDetail->getDstAddr() == &address )
			{
				m_connectionDetailUpdateFlags |= EConnectionUpdateFlags::DST_ADDRESS;
			}
		}

		if ( m_list.empty() )
		{
			return;
		}

		bool mayAffectOrder = false;
		switch ( m_sortMode )
		{
			case EConnectionSortMode::SRC_HOSTNAME:
			case EConnectionSortMode::DST_HOSTNAME:
			case EConnectionSortMode::SRC_COUNTRY:
			case EConnectionSortMode::DST_COUNTRY:
			case EConnectionSortMode::SRC_ASN:
			case EConnectionSortMode::DST_ASN:
			{
				mayAffectOrder = true;
				break;
			}
			default:
			{
				break;
			}
		}

		bool contains = false;
		if ( mayAffectOrder )
		{
			const auto visibleBeginIt = getListBeginIt();
			for ( auto it = m_list.begin(); it != visibleBeginIt; ++it )
			{
				if ( &it->pConnection->getSrcAddr() == &address || &it->pConnection->getDstAddr() == &address )
				{
					contains = true;
				}
			}
		}

		bool isVisible = false;
		for ( auto it = getListBeginIt(); it != m_list.end(); ++it )
		{
			if ( &it->pConnection->getSrcAddr() == &address )
			{
				isVisible = true;
				it->updateFlags |= EConnectionUpdateFlags::SRC_ADDRESS;
				it->isRefreshRequired = true;
			}

			if ( &it->pConnection->getDstAddr() == &address )
			{
				isVisible = true;
				it->updateFlags |= EConnectionUpdateFlags::DST_ADDRESS;
				it->isRefreshRequired = true;
			}
		}

		if ( isVisible )
		{
			contains = true;
			m_isRefreshRequired = true;
		}

		if ( mayAffectOrder )
		{
			if ( contains )
			{
				sort();
			}

			Item item( nullptr );

			for ( auto storageIt = m_storage.begin(); storageIt != m_storage.end(); ++storageIt )
			{
				const ConnectionData *pConnection = &storageIt->second;

				if ( &pConnection->getSrcAddr() == &address || &pConnection->getDstAddr() == &address )
				{
					item.pConnection = pConnection;

					if ( m_compareFunc( item, m_list.back() ) )
					{
						if ( ! containsItem( item ) )
						{
							insertItem( item );
						}
					}
				}
			}
		}
	}

	void ConnectionList::portDataUpdated( const PortData & port )
	{
		if ( m_connectionDetail && m_connectionDetail->hasPorts() )
		{
			if ( &m_connectionDetail->getSrcPort() == &port )
			{
				m_connectionDetailUpdateFlags |= EConnectionUpdateFlags::SRC_PORT;
			}

			if ( &m_connectionDetail->getDstPort() == &port )
			{
				m_connectionDetailUpdateFlags |= EConnectionUpdateFlags::DST_PORT;
			}
		}

		if ( m_list.empty() )
		{
			return;
		}

		bool mayAffectOrder = false;
		switch ( m_sortMode )
		{
			case EConnectionSortMode::SRC_SERVICE:
			case EConnectionSortMode::DST_SERVICE:
			{
				mayAffectOrder = true;
				break;
			}
			default:
			{
				break;
			}
		}

		bool contains = false;
		if ( mayAffectOrder )
		{
			auto visibleBeginIt = getListBeginIt();
			for ( auto it = m_list.begin(); it != visibleBeginIt; ++it )
			{
				if ( ! it->pConnection->hasPorts() )
				{
					continue;
				}

				if ( &it->pConnection->getSrcPort() == &port || &it->pConnection->getDstPort() == &port )
				{
					contains = true;
				}
			}
		}

		bool isVisible = false;
		for ( auto it = getListBeginIt(); it != m_list.end(); ++it )
		{
			if ( ! it->pConnection->hasPorts() )
			{
				continue;
			}

			if ( &it->pConnection->getSrcPort() == &port )
			{
				isVisible = true;
				it->updateFlags |= EConnectionUpdateFlags::SRC_PORT;
				it->isRefreshRequired = true;
			}

			if ( &it->pConnection->getDstPort() == &port )
			{
				isVisible = true;
				it->updateFlags |= EConnectionUpdateFlags::DST_PORT;
				it->isRefreshRequired = true;
			}
		}

		if ( isVisible )
		{
			contains = true;
			m_isRefreshRequired = true;
		}

		if ( mayAffectOrder )
		{
			if ( contains )
			{
				sort();
			}

			Item item( nullptr );

			for ( auto storageIt = m_storage.begin(); storageIt != m_storage.end(); ++storageIt )
			{
				const ConnectionData *pConnection = &storageIt->second;

				if ( ! pConnection->hasPorts() )
				{
					continue;
				}

				if ( &pConnection->getSrcPort() == &port || &pConnection->getDstPort() == &port )
				{
					item.pConnection = pConnection;

					if ( m_compareFunc( item, m_list.back() ) )
					{
						if ( ! containsItem( item ) )
						{
							insertItem( item );
						}
					}
				}
			}
		}
	}

	ConnectionListUpdate ConnectionList::getNextUpdate()
	{
		if ( m_isRefreshRequired )
		{
			unsigned int currentRow = 0;
			for ( auto it = getListBeginIt(); it != m_list.end(); ++it )
			{
				if ( it->isRefreshRequired )
				{
					const bool isCursor = (currentRow == m_cursorPos);
					const int updateFlags = it->updateFlags;

					it->isRefreshRequired = false;
					it->updateFlags = 0;

					return ConnectionListUpdate( it->pConnection, updateFlags, currentRow, isCursor );
				}

				currentRow++;
			}

			// all items in the list are refreshed
			m_isRefreshRequired = false;
		}

		return ConnectionListUpdate();  // empty
	}

	void ConnectionList::invalidateAllVisible()
	{
		for ( auto it = getListBeginIt(); it != m_list.end(); ++it )
		{
			it->isRefreshRequired = true;
			it->updateFlags = -1;
		}

		m_isRefreshRequired = true;
	}

	void ConnectionList::invalidateCursor()
	{
		auto it = getListBeginIt();
		if ( it != m_list.end() )
		{
			it += m_cursorPos;

			it->isRefreshRequired = true;
			it->updateFlags = -1;

			m_isRefreshRequired = true;
		}
	}

	void ConnectionList::moveCursor( int amount )
	{
		if ( amount < 0 )
		{
			amount = -amount;
			const int maxAmount = m_cursorPos;
			if ( amount > maxAmount && m_scrollOffset > 0 )
			{
				m_cursorPos = 0;

				doScroll( maxAmount - amount );  // entire list is invalidated
			}
			else if ( m_cursorPos > 0 )
			{
				if ( amount > maxAmount )
				{
					amount = maxAmount;
				}

				invalidateCursor();
				m_cursorPos -= amount;
				invalidateCursor();
			}
		}
		else if ( amount > 0 )
		{
			const unsigned int maxPos = getCursorMaxPos();
			const int maxAmount = maxPos - m_cursorPos;
			if ( amount > maxAmount && m_storage.getConnectionCount() > m_list.size() )
			{
				m_cursorPos = maxPos;

				doScroll( amount - maxAmount );  // entire list is invalidated
			}
			else if ( m_cursorPos < maxPos )
			{
				if ( amount > maxAmount )
				{
					amount = maxAmount;
				}

				invalidateCursor();
				m_cursorPos += amount;
				invalidateCursor();
			}
		}
	}

	void ConnectionList::doScroll( int amount )
	{
		if ( amount < 0 )
		{
			amount = -amount;

			const int maxAmount = m_scrollOffset;
			if ( amount > maxAmount )
			{
				amount = maxAmount;
			}

			if ( amount > 0 )
			{
				m_scrollOffset -= amount;

				for ( int diff = getSize() - m_targetSize; diff > 0; diff-- )
				{
					m_list.pop_back();
				}

				invalidateAllVisible();
			}
		}
		else if ( amount > 0 )
		{
			const int maxAmount = m_storage.getConnectionCount() - m_list.size();
			if ( amount > maxAmount )
			{
				amount = maxAmount;
			}

			if ( amount > 0 )
			{
				m_scrollOffset += amount;

				invalidateAllVisible();

				fill();
			}
		}
	}

	void ConnectionList::initConnectionDetail()
	{
		auto it = getListBeginIt();
		if ( it != m_list.end() )
		{
			it += m_cursorPos;
			m_connectionDetail = it->pConnection;
		}
		else
		{
			m_connectionDetail = nullptr;
		}

		m_connectionDetailUpdateFlags = -1;
	}

	void ConnectionList::resetConnectionDetailRefresh()
	{
		m_connectionDetailUpdateFlags = 0;
	}

	void ConnectionList::setTargetSize( unsigned int size )
	{
		if ( m_targetSize != size )
		{
			m_targetSize = size;

			const unsigned int currentSize = getSize();
			if ( currentSize > m_targetSize )
			{
				for ( unsigned int diff = currentSize - m_targetSize; diff > 0; diff-- )
				{
					m_list.pop_back();
				}

				m_isRefreshRequired = true;
			}
			else if ( currentSize < m_targetSize )
			{
				fill();
			}

			const unsigned int maxCursorPos = getCursorMaxPos();
			if ( m_cursorPos > maxCursorPos )
			{
				m_cursorPos = maxCursorPos;
				invalidateCursor();
			}
		}
	}

	void ConnectionList::setSortMode( EConnectionSortMode sortMode, bool isAscending )
	{
		if ( m_sortMode != sortMode || m_isSortAscending != isAscending )
		{
			m_sortMode = sortMode;
			m_isSortAscending = isAscending;

			updateCompareFunc();

			m_list.clear();
			fill();

			m_isRefreshRequired = true;
		}
	}

	void ConnectionList::fill()
	{
		if ( m_targetSize <= getSize() || m_storage.getConnectionCount() <= m_list.size() )
		{
			return;
		}

		Item item( nullptr );

		if ( m_list.empty() )
		{
			// there must be at least one connection in the storage (see the above checks)
			item.pConnection = &m_storage.begin()->second;

			m_list.push_back( item );
		}

		for ( auto storageIt = m_storage.begin(); storageIt != m_storage.end(); ++storageIt )
		{
			item.pConnection = &storageIt->second;

			if ( m_compareFunc( item, m_list.back() ) )
			{
				if ( ! containsItem( item ) )
				{
					insertItem( item );
				}
			}
			else if ( getSize() < m_targetSize )
			{
				if ( ! containsItem( item ) )
				{
					m_list.push_back( item );
				}
			}
		}

		m_isRefreshRequired = true;
	}

	void ConnectionList::sort()
	{
		std::sort( m_list.begin(), m_list.end(), m_compareFunc );

		invalidateAllVisible();
	}

	void ConnectionList::insertItem( const Item & item )
	{
		auto it = m_list.insert( std::upper_bound( m_list.begin(), m_list.end(), item, m_compareFunc ), item );

		unsigned int index = std::distance( m_list.begin(), it );

		if ( getSize() > m_targetSize )
		{
			m_list.pop_back();
		}

		const unsigned int visibleBeginIndex = getListBeginIndex();
		if ( index < visibleBeginIndex )
		{
			index = visibleBeginIndex;
		}

		for ( ; index < m_list.size(); ++index )
		{
			m_list[index].isRefreshRequired = true;
			m_list[index].updateFlags = -1;
		}

		m_isRefreshRequired = true;
	}

	void ConnectionList::handleNewConnection( const ConnectionData & connection )
	{
		Item item( &connection );

		if ( ! m_list.empty() && m_compareFunc( item, m_list.back() ) )
		{
			insertItem( item );
		}
		else if ( getSize() < m_targetSize )
		{
			m_list.push_back( item );
			m_isRefreshRequired = true;
		}
	}

	void ConnectionList::handleRemovedConnection( void *pConnection )
	{
		if ( m_connectionDetail == pConnection )
		{
			m_connectionDetail = nullptr;
			m_connectionDetailUpdateFlags = -1;
		}

		auto it = m_list.begin();
		while ( it != m_list.end() )
		{
			if ( pConnection == it->pConnection )
			{
				it = m_list.erase( it );

				break;
			}
			else
			{
				++it;
			}
		}

		const auto visibleBeginIt = getListBeginIt();
		if ( it < visibleBeginIt )
		{
			it = visibleBeginIt;
		}

		// invalidate all visible connections after the removed connection
		for ( ; it != m_list.end(); ++it )
		{
			it->isRefreshRequired = true;
			it->updateFlags = -1;
		}

		fill();

		const unsigned int maxCursorPos = getCursorMaxPos();
		if ( m_cursorPos > maxCursorPos )
		{
			m_cursorPos = maxCursorPos;
			invalidateCursor();
		}

		m_isRefreshRequired = true;
	}

	void ConnectionList::updateCompareFunc()
	{
		switch ( m_sortMode )
		{
			case EConnectionSortMode::NONE:
			{
				m_compareFunc = []( const Item &, const Item & ) -> bool
				{
					return false;
				};
				break;
			}
			case EConnectionSortMode::PROTO:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const EConnectionType aProto = a.pConnection->getType();
						const EConnectionType bProto = b.pConnection->getType();

						return aProto < bProto;
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const EConnectionType aProto = a.pConnection->getType();
						const EConnectionType bProto = b.pConnection->getType();

						return bProto < aProto;
					};
				}
				break;
			}
			case EConnectionSortMode::PROTO_STATE:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const EConnectionType aProto = a.pConnection->getType();
						const EConnectionType bProto = b.pConnection->getType();

						if ( Connection::IsProtoEqual( aProto, bProto ) )
						{
							return a.pConnection->getState() < b.pConnection->getState();
						}
						else
						{
							return aProto < bProto;
						}
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const EConnectionType aProto = a.pConnection->getType();
						const EConnectionType bProto = b.pConnection->getType();

						if ( Connection::IsProtoEqual( aProto, bProto ) )
						{
							return b.pConnection->getState() < a.pConnection->getState();
						}
						else
						{
							return bProto < aProto;
						}
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_ADDRESS:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						if ( aAddr.getAddressType() == bAddr.getAddressType() )
						{
							return aAddr.getNumericString() < bAddr.getNumericString();
						}
						else
						{
							return aAddr.getAddressType() < bAddr.getAddressType();
						}
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						if ( aAddr.getAddressType() == bAddr.getAddressType() )
						{
							return bAddr.getNumericString() < aAddr.getNumericString();
						}
						else
						{
							return bAddr.getAddressType() < aAddr.getAddressType();
						}
					};
				}
				break;
			}
			case EConnectionSortMode::DST_ADDRESS:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						if ( aAddr.getAddressType() == bAddr.getAddressType() )
						{
							return aAddr.getNumericString() < bAddr.getNumericString();
						}
						else
						{
							return aAddr.getAddressType() < bAddr.getAddressType();
						}
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						if ( aAddr.getAddressType() == bAddr.getAddressType() )
						{
							return bAddr.getNumericString() < aAddr.getNumericString();
						}
						else
						{
							return bAddr.getAddressType() < aAddr.getAddressType();
						}
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_HOSTNAME:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return aAddr.getHostnameString() < bAddr.getHostnameString();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return bAddr.getHostnameString() < aAddr.getHostnameString();
					};
				}
				break;
			}
			case EConnectionSortMode::DST_HOSTNAME:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return aAddr.getHostnameString() < bAddr.getHostnameString();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return bAddr.getHostnameString() < aAddr.getHostnameString();
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_ASN:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return aAddr.getASN() < bAddr.getASN();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return bAddr.getASN() < aAddr.getASN();
					};
				}
				break;
			}
			case EConnectionSortMode::DST_ASN:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return aAddr.getASN() < bAddr.getASN();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return bAddr.getASN() < aAddr.getASN();
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_COUNTRY:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return aAddr.getCountry() < bAddr.getCountry();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getSrcAddr();
						const AddressData & bAddr = b.pConnection->getSrcAddr();

						return bAddr.getCountry() < aAddr.getCountry();
					};
				}
				break;
			}
			case EConnectionSortMode::DST_COUNTRY:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return aAddr.getCountry() < bAddr.getCountry();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						const AddressData & aAddr = a.pConnection->getDstAddr();
						const AddressData & bAddr = b.pConnection->getDstAddr();

						return bAddr.getCountry() < aAddr.getCountry();
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_PORT:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getSrcPort();
						const PortData & bPort = b.pConnection->getSrcPort();

						return aPort.getPortNumber() < bPort.getPortNumber();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getSrcPort();
						const PortData & bPort = b.pConnection->getSrcPort();

						return bPort.getPortNumber() < aPort.getPortNumber();
					};
				}
				break;
			}
			case EConnectionSortMode::DST_PORT:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getDstPort();
						const PortData & bPort = b.pConnection->getDstPort();

						return aPort.getPortNumber() < bPort.getPortNumber();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getDstPort();
						const PortData & bPort = b.pConnection->getDstPort();

						return bPort.getPortNumber() < aPort.getPortNumber();
					};
				}
				break;
			}
			case EConnectionSortMode::SRC_SERVICE:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getSrcPort();
						const PortData & bPort = b.pConnection->getSrcPort();

						return aPort.getServiceString() < bPort.getServiceString();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getSrcPort();
						const PortData & bPort = b.pConnection->getSrcPort();

						return bPort.getServiceString() < aPort.getServiceString();
					};
				}
				break;
			}
			case EConnectionSortMode::DST_SERVICE:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getDstPort();
						const PortData & bPort = b.pConnection->getDstPort();

						return aPort.getServiceString() < bPort.getServiceString();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						if ( ! a.pConnection->hasPorts() || ! b.pConnection->hasPorts() )
						{
							return false;
						}

						const PortData & aPort = a.pConnection->getDstPort();
						const PortData & bPort = b.pConnection->getDstPort();

						return bPort.getServiceString() < aPort.getServiceString();
					};
				}
				break;
			}
			case EConnectionSortMode::RX_PACKETS:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getRXPackets() < b.pConnection->getRXPackets();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getRXPackets() < a.pConnection->getRXPackets();
					};
				}
				break;
			}
			case EConnectionSortMode::TX_PACKETS:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getTXPackets() < b.pConnection->getTXPackets();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getTXPackets() < a.pConnection->getTXPackets();
					};
				}
				break;
			}
			case EConnectionSortMode::RX_BYTES:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getRXBytes() < b.pConnection->getRXBytes();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getRXBytes() < a.pConnection->getRXBytes();
					};
				}
				break;
			}
			case EConnectionSortMode::TX_BYTES:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getTXBytes() < b.pConnection->getTXBytes();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getTXBytes() < a.pConnection->getTXBytes();
					};
				}
				break;
			}
			case EConnectionSortMode::RX_SPEED:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getRXSpeed() < b.pConnection->getRXSpeed();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getRXSpeed() < a.pConnection->getRXSpeed();
					};
				}
				break;
			}
			case EConnectionSortMode::TX_SPEED:
			{
				if ( m_isSortAscending )
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return a.pConnection->getTXSpeed() < b.pConnection->getTXSpeed();
					};
				}
				else
				{
					m_compareFunc = []( const Item & a, const Item & b ) -> bool
					{
						return b.pConnection->getTXSpeed() < a.pConnection->getTXSpeed();
					};
				}
				break;
			}
		}
	}

	void ConnectionList::ResolverCallbackAddress( AddressData & address, ResolvedAddress & resolved, void *param )
	{
		ConnectionList *self = static_cast<ConnectionList*>( param );

		address.setResolvedHostname( std::move( resolved.hostname ) );
		address.setResolvedCountry( std::move( resolved.country ) );
		address.setResolvedASN( std::move( resolved.asn ) );

		self->m_dataResolvedCount++;
		self->addressDataUpdated( address );

		gApp->getUI()->refreshConnectionList();
	}

	void ConnectionList::ResolverCallbackPort( PortData & port, ResolvedPort & resolved, void *param )
	{
		ConnectionList *self = static_cast<ConnectionList*>( param );

		port.setResolvedService( std::move( resolved.service ) );

		self->m_dataResolvedCount++;
		self->portDataUpdated( port );

		gApp->getUI()->refreshConnectionList();
	}
}
