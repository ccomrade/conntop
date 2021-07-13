/**
 * @file
 * @brief ConnectionStorage class.
 */

#pragma once

#include <unordered_map>

#include "Connection.hpp"
#include "Address.hpp"
#include "Port.hpp"

class ConnectionStorage
{
public:
	using ConnectionMapType = std::unordered_map<Connection, ConnectionData>;
	using AddressIP4MapType = std::unordered_map<AddressIP4, AddressData>;
	using AddressIP6MapType = std::unordered_map<AddressIP6, AddressData>;
	using PortMapType = std::unordered_map<Port, PortData>;

private:
	ConnectionMapType m_connectionMap;
	AddressIP4MapType m_addressIP4Map;
	AddressIP6MapType m_addressIP6Map;
	PortMapType m_portMap;

public:
	ConnectionStorage()
	: m_connectionMap(),
	  m_addressIP4Map(),
	  m_addressIP6Map(),
	  m_portMap()
	{
	}

	std::pair<AddressData*, bool> addIP4Address(const AddressIP4 & address)
	{
		auto result = m_addressIP4Map.emplace(address, address);
		auto it = result.first;
		bool isNew = result.second;
		AddressData *pData = &it->second;
		if (isNew)
		{
			pData->setAddress(it->first);
		}
		return { pData, isNew };
	}

	std::pair<AddressData*, bool> addIP6Address(const AddressIP6 & address)
	{
		auto result = m_addressIP6Map.emplace(address, address);
		auto it = result.first;
		bool isNew = result.second;
		AddressData *pData = &it->second;
		if (isNew)
		{
			pData->setAddress(it->first);
		}
		return { pData, isNew };
	}

	std::pair<PortData*, bool> addPort(const Port & port)
	{
		auto result = m_portMap.emplace(port, port);
		auto it = result.first;
		bool isNew = result.second;
		PortData *pData = &it->second;
		if (isNew)
		{
			pData->setPort(it->first);
		}
		return { pData, isNew };
	}

	template<class... Args>
	std::pair<ConnectionData*, bool> addConnection(const Connection & c, Args &&... args)
	{
		auto result = m_connectionMap.emplace(c, ConnectionData(c, std::forward<Args>(args)...));
		auto it = result.first;
		bool isNew = result.second;
		ConnectionData *pData = &it->second;
		if (isNew)
		{
			pData->setConnection(it->first);
		}
		return { pData, isNew };
	}

	void *removeConnection(const Connection & connection)
	{
		void *pData = nullptr;
		auto it = m_connectionMap.find(connection);
		if (it != m_connectionMap.end())
		{
			pData = &it->second;
			m_connectionMap.erase(it);
		}
		return pData;
	}

	void clearConnections()
	{
		m_connectionMap.clear();
	}

	ConnectionData *getConnection(const Connection & connection)
	{
		auto it = m_connectionMap.find(connection);
		return (it != m_connectionMap.end()) ? &it->second : nullptr;
	}

	AddressData *getIP4Address(const AddressIP4 & address)
	{
		auto it = m_addressIP4Map.find(address);
		return (it != m_addressIP4Map.end()) ? &it->second : nullptr;
	}

	AddressData *getIP6Address(const AddressIP6 & address)
	{
		auto it = m_addressIP6Map.find(address);
		return (it != m_addressIP6Map.end()) ? &it->second : nullptr;
	}

	PortData *getPort(const Port & port)
	{
		auto it = m_portMap.find(port);
		return (it != m_portMap.end()) ? &it->second : nullptr;
	}

	ConnectionMapType::iterator begin()
	{
		return m_connectionMap.begin();
	}

	ConnectionMapType::iterator end()
	{
		return m_connectionMap.end();
	}

	ConnectionMapType::const_iterator begin() const
	{
		return m_connectionMap.cbegin();
	}

	ConnectionMapType::const_iterator end() const
	{
		return m_connectionMap.cend();
	}

	ConnectionMapType::const_iterator cbegin() const
	{
		return m_connectionMap.cbegin();
	}

	ConnectionMapType::const_iterator cend() const
	{
		return m_connectionMap.cend();
	}

	std::pair<AddressIP4MapType::iterator, AddressIP4MapType::iterator> getIP4AddressIterators()
	{
		return { m_addressIP4Map.begin(), m_addressIP4Map.end() };
	}

	std::pair<AddressIP6MapType::iterator, AddressIP6MapType::iterator> getIP6AddressIterators()
	{
		return { m_addressIP6Map.begin(), m_addressIP6Map.end() };
	}

	std::pair<PortMapType::iterator, PortMapType::iterator> getPortIterators()
	{
		return { m_portMap.begin(), m_portMap.end() };
	}

	size_t getConnectionCount() const
	{
		return m_connectionMap.size();
	}

	size_t getIP4AddressCount() const
	{
		return m_addressIP4Map.size();
	}

	size_t getIP6AddressCount() const
	{
		return m_addressIP6Map.size();
	}

	size_t getPortCount() const
	{
		return m_portMap.size();
	}
};
