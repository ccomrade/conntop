#pragma once

#include <stdint.h>
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
	using Value = std::array<uint8_t, 16>;

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

	// Creation
	static constexpr IPAddress ZeroIPv4();
	static constexpr IPAddress ZeroIPv6();

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

constexpr unsigned int IPAddress::GetValueSize() const
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

//////////////////////////////
// Creation of IP addresses //
//////////////////////////////

constexpr IPAddress IPAddress::ZeroIPv4()
{
	IPAddress result;
	result.type = IPAddressType::IPv4;

	return result;
}

constexpr IPAddress IPAddress::ZeroIPv6()
{
	IPAddress result;
	result.type = IPAddressType::IPv6;

	return result;
}

constexpr IPAddress IPAddress::FromString(const std::string_view& string)
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

constexpr IPAddress IPAddress::FromStringIPv4(const std::string_view& string)
{
	IPAddress result;
	result.type = IPAddressType::IPv4;

	if (!ParseIPv4(string, result.value))
	{
		throw std::invalid_argument("Invalid IPv4 address '" + std::string(string) + "'");
	}

	return result;
}

constexpr IPAddress IPAddress::FromStringIPv6(const std::string_view& string)
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

constexpr bool IPAddress::ParseIPv4(const std::string_view& string, IPAddress::Value& result)
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

		result[byteIndex] = static_cast<uint8_t>(byteValue);

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

constexpr bool IPAddress::ParseIPv6(const std::string_view& string, IPAddress::Value& result)
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
			*it = static_cast<uint8_t>(fieldValue >> 8);
			++it;
			*it = static_cast<uint8_t>(fieldValue);
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

constexpr int IPAddress::Compare(const IPAddress& other) const
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

constexpr bool IPAddress::IsEqual(const IPAddress& other) const
{
	return Compare(other) == 0;
}

constexpr bool IPAddress::IsInRange(const IPAddress& first, const IPAddress& last) const
{
	return Compare(first) >= 0 && Compare(last) <= 0;
}

//////////////////////////
// Comparison operators //
//////////////////////////

constexpr bool operator==(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) == 0;
}

constexpr bool operator!=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) != 0;
}

constexpr bool operator<=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) <= 0;
}

constexpr bool operator>=(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) >= 0;
}

constexpr bool operator<(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) < 0;
}

constexpr bool operator>(const IPAddress& a, const IPAddress& b)
{
	return a.Compare(b) > 0;
}

///////////////////////////
// Reserved IP addresses //
///////////////////////////

constexpr bool IPAddress::IsZero() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			constexpr IPAddress ZERO_V4 = IPAddress::FromStringIPv4("0.0.0.0");

			return IsEqual(ZERO_V4);
		}
		case IPAddressType::IPv6:
		{
			constexpr IPAddress ZERO_V6 = IPAddress::FromStringIPv6("::");

			return IsEqual(ZERO_V6);
		}
	}

	return false;
}

constexpr bool IPAddress::IsLoopBack() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			constexpr IPAddress LOOP_BACK_V4_A = IPAddress::FromStringIPv4("127.0.0.0");
			constexpr IPAddress LOOP_BACK_V4_B = IPAddress::FromStringIPv4("127.255.255.255");

			return IsInRange(LOOP_BACK_V4_A, LOOP_BACK_V4_B);
		}
		case IPAddressType::IPv6:
		{
			constexpr IPAddress LOOP_BACK_V6 = IPAddress::FromStringIPv6("::1");

			return IsEqual(LOOP_BACK_V6);
		}
	}

	return false;
}

constexpr bool IPAddress::IsLinkLocal() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			constexpr IPAddress LINK_LOCAL_V4_A = IPAddress::FromStringIPv4("169.254.0.0");
			constexpr IPAddress LINK_LOCAL_V4_B = IPAddress::FromStringIPv4("169.254.255.255");

			return IsInRange(LINK_LOCAL_V4_A, LINK_LOCAL_V4_B);
		}
		case IPAddressType::IPv6:
		{
			constexpr IPAddress LINK_LOCAL_V6_A = IPAddress::FromStringIPv6("fe80::");
			constexpr IPAddress LINK_LOCAL_V6_B = IPAddress::FromStringIPv6("fe80::ffff:ffff:ffff:ffff");

			return IsInRange(LINK_LOCAL_V6_A, LINK_LOCAL_V6_B);
		}
	}

	return false;
}

constexpr bool IPAddress::IsPrivate() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			constexpr IPAddress PRIVATE_V4_1_A = IPAddress::FromStringIPv4("10.0.0.0");
			constexpr IPAddress PRIVATE_V4_1_B = IPAddress::FromStringIPv4("10.255.255.255");

			constexpr IPAddress PRIVATE_V4_2_A = IPAddress::FromStringIPv4("172.16.0.0");
			constexpr IPAddress PRIVATE_V4_2_B = IPAddress::FromStringIPv4("172.31.255.255");

			constexpr IPAddress PRIVATE_V4_3_A = IPAddress::FromStringIPv4("192.168.0.0");
			constexpr IPAddress PRIVATE_V4_3_B = IPAddress::FromStringIPv4("192.168.255.255");

			return IsInRange(PRIVATE_V4_1_A, PRIVATE_V4_1_B) ||
			       IsInRange(PRIVATE_V4_2_A, PRIVATE_V4_2_B) ||
			       IsInRange(PRIVATE_V4_3_A, PRIVATE_V4_3_B);
		}
		case IPAddressType::IPv6:
		{
			constexpr IPAddress PRIVATE_V6_A = IPAddress::FromStringIPv6("fc00::");
			constexpr IPAddress PRIVATE_V6_B = IPAddress::FromStringIPv6("fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

			return IsInRange(PRIVATE_V6_A, PRIVATE_V6_B);
		}
	}

	return false;
}

constexpr bool IPAddress::IsMulticast() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
		{
			constexpr IPAddress MULTICAST_V4_A = IPAddress::FromStringIPv4("224.0.0.0");
			constexpr IPAddress MULTICAST_V4_B = IPAddress::FromStringIPv4("239.255.255.255");

			return IsInRange(MULTICAST_V4_A, MULTICAST_V4_B);
		}
		case IPAddressType::IPv6:
		{
			constexpr IPAddress MULTICAST_V6_A = IPAddress::FromStringIPv6("ff00::");
			constexpr IPAddress MULTICAST_V6_B = IPAddress::FromStringIPv6("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

			return IsInRange(MULTICAST_V6_A, MULTICAST_V6_B);
		}
	}

	return false;
}