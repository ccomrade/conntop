#pragma once

#include <cstdint>
#include <cstring>
#include <array>
#include <string>
#include <string_view>
#include <stdexcept>

enum class IPAddressType
{
	IPv4, IPv6
};

struct IPAddress
{
	using Value = std::array<std::uint8_t, 16>;

	///////////////
	// Variables //
	///////////////

	IPAddressType type = IPAddressType::IPv6;
	Value value = {};

	///////////////
	// Functions //
	///////////////

	constexpr IPAddress() = default;

	constexpr unsigned int GetValueSize() const;

	void CopyTo(void* buffer) const;

	void CopyFrom(const void* buffer);
	void CopyFrom(const void* buffer, IPAddressType newType);

	// Creation
	static constexpr IPAddress FromString(const std::string_view& string);
	static constexpr IPAddress FromStringIPv4(const std::string_view& string);
	static constexpr IPAddress FromStringIPv6(const std::string_view& string);

	// String conversion
	static constexpr bool ParseIPv4(const std::string_view& string, IPAddress::Value& result);
	static constexpr bool ParseIPv6(const std::string_view& string, IPAddress::Value& result);

	std::string ToString() const;

	// Comparison
	constexpr int Compare(const IPAddress& other) const;

	constexpr bool IsEqual(const IPAddress& other) const;
	constexpr bool IsInRange(const IPAddress& first, const IPAddress& last) const;

	// Reserved IP addresses
	constexpr bool IsZero() const;
	constexpr bool IsLoopBack() const;
	constexpr bool IsLinkLocal() const;
	constexpr bool IsPrivate() const;
	constexpr bool IsMulticast() const;
};

////////////////////
// Implementation //
////////////////////

inline constexpr unsigned int IPAddress::GetValueSize() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
			return 4;
		case IPAddressType::IPv6:
			return 16;
	}

	return 0;
}

inline void IPAddress::CopyTo(void* buffer) const
{
	if (buffer)
	{
		std::memcpy(buffer, value.data(), GetValueSize());
	}
}

inline void IPAddress::CopyFrom(const void* buffer)
{
	if (buffer)
	{
		std::memcpy(value.data(), buffer, GetValueSize());
	}
}

inline void IPAddress::CopyFrom(const void* buffer, IPAddressType newType)
{
	type = newType;
	CopyFrom(buffer);
}

//////////////////////////////
// Creation of IP addresses //
//////////////////////////////

inline constexpr IPAddress IPAddress::FromString(const std::string_view& string)
{
	IPAddress result;

	if (ParseIPv4(string, result.value))
	{
		result.type = IPAddressType::IPv4;
	}
	else if (ParseIPv6(string, result.value))
	{
		result.type = IPAddressType::IPv6;
	}
	else
	{
		throw std::invalid_argument("Invalid IP address '" + std::string(string) + "'");
	}

	return result;
}

inline constexpr IPAddress IPAddress::FromStringIPv4(const std::string_view& string)
{
	IPAddress result;
	result.type = IPAddressType::IPv4;

	if (!ParseIPv4(string, result.value))
	{
		throw std::invalid_argument("Invalid IPv4 address '" + std::string(string) + "'");
	}

	return result;
}

inline constexpr IPAddress IPAddress::FromStringIPv6(const std::string_view& string)
{
	IPAddress result;
	result.type = IPAddressType::IPv6;

	if (!ParseIPv6(string, result.value))
	{
		throw std::invalid_argument("Invalid IPv6 address '" + std::string(string) + "'");
	}

	return result;
}

////////////////////////////
// IP address from string //
////////////////////////////

inline constexpr bool IPAddress::ParseIPv4(const std::string_view& string, IPAddress::Value& result)
{
	auto stringIt = string.begin();

	for (unsigned int byteIndex = 0; byteIndex < 4; byteIndex++)
	{
		unsigned int byteValue = 0;
		unsigned int digitCount = 0;

		// parse byte value
		while (digitCount < 3 && stringIt != string.end())
		{
			const char ch = *stringIt;
			unsigned int digit = 0;

			if (ch >= '0' && ch <= '9')
				digit = ch - '0';
			else
				break;

			byteValue = (byteValue * 10) + digit;

			++digitCount;
			++stringIt;
		}

		if (digitCount == 0 || byteValue > 255)
			return false;

		result[byteIndex] = static_cast<std::uint8_t>(byteValue);

		// there must be a dot after each byte except the last one
		if (byteIndex < 3)
		{
			if (stringIt != string.end() && *stringIt == '.')
				++stringIt;
			else
				return false;
		}
	}

	// make sure the entire string has been parsed
	return stringIt == string.end();
}

inline constexpr bool IPAddress::ParseIPv6(const std::string_view& string, IPAddress::Value& result)
{
	unsigned int colonCount = 0;

	for (char ch : string)
	{
		if (ch == ':')
			colonCount++;
	}

	if (colonCount < 2 || colonCount > 7)
		return false;

	auto stringIt = string.begin();
	bool doubleColonFound = false;

	for (auto it = result.begin(); it != result.end();)
	{
		unsigned int fieldValue = 0;
		unsigned int digitCount = 0;

		// parse field value
		while (digitCount < 4 && stringIt != string.end())
		{
			const char ch = *stringIt;
			unsigned int digit = 0;

			if (ch >= '0' && ch <= '9')
				digit = ch - '0';
			else if (ch >= 'a' && ch <= 'f')
				digit = (ch - 'a') + 10;
			else if (ch >= 'A' && ch <= 'F')
				digit = (ch - 'A') + 10;
			else
				break;

			fieldValue = (fieldValue * 16) + digit;

			++digitCount;
			++stringIt;
		}

		if (digitCount > 0)
		{
			*it = static_cast<std::uint8_t>(fieldValue >> 8);
			++it;
			*it = static_cast<std::uint8_t>(fieldValue);
			++it;
		}

		unsigned int expectedColonCount = 0;

		// there must be a colon after each field except the last one
		if (it != result.end())
			expectedColonCount++;

		// the address begins with double colon
		if (digitCount == 0 && it == result.begin())
			expectedColonCount++;

		for (unsigned int i = 0; i < expectedColonCount; i++)
		{
			if (stringIt != string.end() && *stringIt == ':')
				++stringIt;
			else
				return false;
		}

		// double colon
		if (digitCount == 0)
		{
			if (!doubleColonFound)
				doubleColonFound = true;
			else
				return false;

			unsigned int hiddenZeroCount = 8 - colonCount;

			// the address begins with double colon
			if (it == result.begin())
				hiddenZeroCount++;

			// the address ends with double colon
			if (stringIt == string.end())
				hiddenZeroCount++;

			// expand double colon
			for (unsigned int i = 0; i < hiddenZeroCount; i++)
			{
				*it = 0;
				++it;
				*it = 0;
				++it;
			}
		}
	}

	// make sure the entire string has been parsed
	return stringIt == string.end();
}

////////////////////////////////
// Comparison of IP addresses //
////////////////////////////////

inline constexpr int IPAddress::Compare(const IPAddress& other) const
{
	if (type != other.type)
	{
		return (type < other.type) ? -1 : 1;
	}
	else
	{
		const unsigned int valueSize = GetValueSize();

		for (unsigned int i = 0; i < valueSize; i++)
		{
			if (value[i] != other.value[i])
			{
				return (value[i] < other.value[i]) ? -1 : 1;
			}
		}

		return 0;
	}
}

inline constexpr bool IPAddress::IsEqual(const IPAddress& other) const
{
	return Compare(other) == 0;
}

inline constexpr bool IPAddress::IsInRange(const IPAddress& first, const IPAddress& last) const
{
	return Compare(first) >= 0 && Compare(last) <= 0;
}

//////////////////////////
// Comparison operators //
//////////////////////////

inline constexpr bool operator==(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) == 0;
}

inline constexpr bool operator!=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) != 0;
}

inline constexpr bool operator<=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) <= 0;
}

inline constexpr bool operator>=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) >= 0;
}

inline constexpr bool operator<(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) < 0;
}

inline constexpr bool operator>(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) > 0;
}

///////////////////////////
// Reserved IP addresses //
///////////////////////////

namespace ReservedIPAddresses
{
	namespace IPv4
	{
		inline constexpr IPAddress ZERO = IPAddress::FromStringIPv4("0.0.0.0");

		inline constexpr IPAddress LOOPBACK_FIRST = IPAddress::FromStringIPv4("127.0.0.0");
		inline constexpr IPAddress LOOPBACK_LAST  = IPAddress::FromStringIPv4("127.255.255.255");

		inline constexpr IPAddress LINK_LOCAL_FIRST = IPAddress::FromStringIPv4("169.254.0.0");
		inline constexpr IPAddress LINK_LOCAL_LAST  = IPAddress::FromStringIPv4("169.254.255.255");

		inline constexpr IPAddress PRIVATE_A_FIRST = IPAddress::FromStringIPv4("10.0.0.0");
		inline constexpr IPAddress PRIVATE_A_LAST  = IPAddress::FromStringIPv4("10.255.255.255");
		inline constexpr IPAddress PRIVATE_B_FIRST = IPAddress::FromStringIPv4("172.16.0.0");
		inline constexpr IPAddress PRIVATE_B_LAST  = IPAddress::FromStringIPv4("172.31.255.255");
		inline constexpr IPAddress PRIVATE_C_FIRST = IPAddress::FromStringIPv4("192.168.0.0");
		inline constexpr IPAddress PRIVATE_C_LAST  = IPAddress::FromStringIPv4("192.168.255.255");

		inline constexpr IPAddress MULTICAST_FIRST = IPAddress::FromStringIPv4("224.0.0.0");
		inline constexpr IPAddress MULTICAST_LAST  = IPAddress::FromStringIPv4("239.255.255.255");
	}

	namespace IPv6
	{
		inline constexpr IPAddress ZERO = IPAddress::FromStringIPv6("::");

		inline constexpr IPAddress LOOPBACK = IPAddress::FromStringIPv6("::1");

		inline constexpr IPAddress LINK_LOCAL_FIRST = IPAddress::FromStringIPv6("fe80::");
		inline constexpr IPAddress LINK_LOCAL_LAST  = IPAddress::FromStringIPv6("fe80::ffff:ffff:ffff:ffff");

		inline constexpr IPAddress PRIVATE_FIRST = IPAddress::FromStringIPv6("fc00::");
		inline constexpr IPAddress PRIVATE_LAST  = IPAddress::FromStringIPv6("fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

		inline constexpr IPAddress MULTICAST_FIRST = IPAddress::FromStringIPv6("ff00::");
		inline constexpr IPAddress MULTICAST_LAST  = IPAddress::FromStringIPv6("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
	}
}

inline constexpr bool IPAddress::IsZero() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			return IsEqual(ReservedIPAddresses::IPv4::ZERO);
		}
		case IPAddressType::IPv6:
		{
			return IsEqual(ReservedIPAddresses::IPv6::ZERO);
		}
	}

	return false;
}

inline constexpr bool IPAddress::IsLoopBack() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			return IsInRange(ReservedIPAddresses::IPv4::LOOPBACK_FIRST, ReservedIPAddresses::IPv4::LOOPBACK_LAST);
		}
		case IPAddressType::IPv6:
		{
			return IsEqual(ReservedIPAddresses::IPv6::LOOPBACK);
		}
	}

	return false;
}

inline constexpr bool IPAddress::IsLinkLocal() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			return IsInRange(ReservedIPAddresses::IPv4::LINK_LOCAL_FIRST, ReservedIPAddresses::IPv4::LINK_LOCAL_LAST);
		}
		case IPAddressType::IPv6:
		{
			return IsInRange(ReservedIPAddresses::IPv6::LINK_LOCAL_FIRST, ReservedIPAddresses::IPv6::LINK_LOCAL_LAST);
		}
	}

	return false;
}

inline constexpr bool IPAddress::IsPrivate() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			return IsInRange(ReservedIPAddresses::IPv4::PRIVATE_A_FIRST, ReservedIPAddresses::IPv4::PRIVATE_A_LAST)
			    || IsInRange(ReservedIPAddresses::IPv4::PRIVATE_B_FIRST, ReservedIPAddresses::IPv4::PRIVATE_B_LAST)
			    || IsInRange(ReservedIPAddresses::IPv4::PRIVATE_C_FIRST, ReservedIPAddresses::IPv4::PRIVATE_C_LAST);
		}
		case IPAddressType::IPv6:
		{
			return IsInRange(ReservedIPAddresses::IPv6::PRIVATE_FIRST, ReservedIPAddresses::IPv6::PRIVATE_LAST);
		}
	}

	return false;
}

inline constexpr bool IPAddress::IsMulticast() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			return IsInRange(ReservedIPAddresses::IPv4::MULTICAST_FIRST, ReservedIPAddresses::IPv4::MULTICAST_LAST);
		}
		case IPAddressType::IPv6:
		{
			return IsInRange(ReservedIPAddresses::IPv6::MULTICAST_FIRST, ReservedIPAddresses::IPv6::MULTICAST_LAST);
		}
	}

	return false;
}
