/**
 * @file
 * @brief Implementation of platform-specific utilities for Unix platform.
 */

#include <string.h>
#include <time.h>
#include <mutex>

#include "Util.hpp"
#include "Log.hpp"

static std::mutex g_localtime_mutex;

std::string Util::ErrnoToString(int errorNumber)
{
	char buffer[256];

#ifdef _GNU_SOURCE
	const char *msg = strerror_r(errorNumber, buffer, sizeof buffer);  // GNU-specific version
	if (msg)
	{
		return std::string(msg);
	}
#else
	int status = strerror_r(errorNumber, buffer, sizeof buffer);  // POSIX version
	if (status == 0)
	{
		return std::string(buffer);
	}
#endif

	if (gLog)
	{
		gLog->error("[Platform] strerror_r failed, error number = %d", errorNumber);
	}

	return std::string();
}

DateTime Util::UnixTimeToDateTimeUTC(const UnixTime & unixTime)
{
	if (unixTime.isEmpty())
	{
		return DateTime();
	}

	time_t secs = unixTime.getSeconds();
	tm convertedTime;
	if (gmtime_r(&secs, &convertedTime) == nullptr)
	{
		if (gLog)
		{
			gLog->error("[Platform] gmtime_r failed: %s", Util::ErrnoToString().c_str());
		}
		return DateTime();
	}

	const int year = convertedTime.tm_year + 1900;
	const int month = convertedTime.tm_mon + 1;
	const int day = convertedTime.tm_mday;
	const int hour = convertedTime.tm_hour;
	const int minute = convertedTime.tm_min;
	const int second = convertedTime.tm_sec;

	const TimeZone tz;  // UTC

	return DateTime(year, month, day, hour, minute, second, unixTime.getMilliseconds(), tz);
}

DateTime Util::UnixTimeToDateTimeLocal(const UnixTime & unixTime)
{
	if (unixTime.isEmpty())
	{
		return DateTime();
	}

	// thread-safe version of localtime doesn't call tzset...
	// also, tzset itself is not so thread-safe because it sets global variables
	std::lock_guard<std::mutex> lock(g_localtime_mutex);

	time_t secs = unixTime.getSeconds();
	tm *pConvertedTime = localtime(&secs);
	if (pConvertedTime == nullptr)
	{
		if (gLog)
		{
			gLog->error("[Platform] localtime failed: %s", Util::ErrnoToString().c_str());
		}
		return DateTime();
	}

	const int year = pConvertedTime->tm_year + 1900;
	const int month = pConvertedTime->tm_mon + 1;
	const int day = pConvertedTime->tm_mday;
	const int hour = pConvertedTime->tm_hour;
	const int minute = pConvertedTime->tm_min;
	const int second = pConvertedTime->tm_sec;

	long tzSeconds = -timezone;  // global variable that holds seconds West of UTC initialized by tzset
	bool tzNegative = false;
	if (tzSeconds < 0)
	{
		tzNegative = true;
		tzSeconds = -tzSeconds;
	}

	const TimeZone tz(tzNegative, tzSeconds / 3600, (tzSeconds % 3600) / 60);

	return DateTime(year, month, day, hour, minute, second, unixTime.getMilliseconds(), tz);
}
