/**
 * @file
 * @brief Utilities.
 */

#pragma once

#include <cerrno>
#include <string>

#include "Types.hpp"
#include "DateTime.hpp"

namespace ctp
{
	class IAddress;
	class Port;

	namespace Util
	{
		std::string GetHumanReadableSize(uint64_t bytes);

		std::string AddressPortToString(const IAddress & address, uint16_t portNumber);
		std::string AddressPortToString(const IAddress & address, const Port & port);

		void LogMemoryUsage(bool always = false);

		/**
		 * @brief Obtains text description of error number (errno).
		 * Implementation is platform-specific.
		 * @param errorNumber The error number.
		 * @return String with error description or empty string if the description cannot be obtained.
		 */
		std::string ErrnoToString(int errorNumber = errno);

		/**
		 * @brief Converts Unix time to UTC date and time.
		 * Implementation is platform-specific.
		 * @param unixTime Unix time to convert.
		 * @return UTC date and time.
		 */
		DateTime UnixTimeToDateTimeUTC(const UnixTime & unixTime);

		/**
		 * @brief Converts Unix time to local date and time.
		 * Implementation is platform-specific.
		 * @param unixTime Unix time to convert.
		 * @return Local date and time.
		 */
		DateTime UnixTimeToDateTimeLocal(const UnixTime & unixTime);
	}
}
