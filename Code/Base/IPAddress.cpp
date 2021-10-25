#include "IPAddress.h"

//////////////////////////
// IP address to string //
//////////////////////////

namespace
{
	std::string ToStringIPv4(const IPAddress::Value& address)
	{
		std::string result;

		// prepare space for the longest possible IPv4 address string
		result.reserve(15);

		for (int i = 0; i < 4; i++)
		{
			result += std::to_string(address[i]);

			if (i < 3)
				result += '.';
		}

		return result;
	}

	std::string ToStringIPv6(const IPAddress::Value& address)
	{
		std::string result;

		// prepare space for the longest possible IPv6 address string
		result.reserve(39);

		// find the longest sequence of zero fields
		auto doubleColonBeginIt = address.end();
		auto doubleColonEndIt = address.end();

		for (auto it = address.begin(); it != address.end();)
		{
			if (*it || *(it + 1))
			{
				it += 2;
			}
			else
			{
				const auto zerosBeginIt = it;

				for (it += 2; it != address.end(); it += 2)
				{
					if (*it || *(it + 1))
						break;
				}

				const auto zerosLength = it - zerosBeginIt;

				// at least 2 consecutive zero fields
				if (zerosLength >= (2 * 2) && zerosLength > (doubleColonEndIt - doubleColonBeginIt))
				{
					doubleColonBeginIt = zerosBeginIt;
					doubleColonEndIt = it;
				}
			}
		}

		// and now convert the address to string
		for (auto it = address.begin(); it != address.end();)
		{
			if (it == doubleColonBeginIt)
			{
				result += ':';

				if (it == address.begin())
					result += ':';

				it = doubleColonEndIt;
			}
			else
			{
				const uint8_t hiByte = *it;
				const uint8_t loByte = *(it + 1);

				it += 2;

				constexpr std::string_view DIGITS = "0123456789abcdef";

				// skip leading zeros
				if (hiByte > 0xF)
					result += DIGITS[hiByte >> 4];

				if (hiByte)
					result += DIGITS[hiByte & 0xF];

				if (hiByte || loByte > 0xF)
					result += DIGITS[loByte >> 4];

				// the last digit is always there
				result += DIGITS[loByte & 0xF];

				if (it != address.end())
					result += ':';
			}
		}

		return result;
	}
}

std::string IPAddress::ToString() const
{
	switch (type)
	{
		case IPAddressType::IPv4:
			return ToStringIPv4(value);
		case IPAddressType::IPv6:
			return ToStringIPv6(value);
	}

	return {};
}
