/**
 * @file
 * @brief Classes for storing date and time information.
 */

#pragma once

#include <string>

#include "Types.hpp"

namespace ctp
{
	class TimeZone
	{
		bool m_isNegative;
		uint8_t m_hourOffset;    //!< 0..12
		uint8_t m_minuteOffset;  //!< 0..59

	public:
		TimeZone()
		: m_isNegative(false),
		  m_hourOffset(0),
		  m_minuteOffset(0)
		{
		}

		TimeZone(bool isNegative, uint8_t hourOffset, uint8_t minuteOffset = 0)
		: m_isNegative(isNegative),
		  m_hourOffset(hourOffset),
		  m_minuteOffset(minuteOffset)
		{
		}

		bool isNegative() const
		{
			return m_isNegative;
		}

		bool isUTC() const
		{
			return m_isNegative == false && m_hourOffset == 0 && m_minuteOffset == 0;
		}

		uint8_t getHourOffset() const
		{
			return m_hourOffset;
		}

		uint8_t getMinuteOffset() const
		{
			return m_minuteOffset;
		}
	};

	class DateTime
	{
	public:
		enum EType
		{
			UTC,
			LOCAL
		};

	private:
		int32_t m_year;         //!< 1..INT32_MAX
		int16_t m_month;        //!< 1..12
		int16_t m_day;          //!< 1..31
		int16_t m_hour;         //!< 00..23
		int16_t m_minute;       //!< 00..59
		int16_t m_second;       //!< 00..60
		int16_t m_millisecond;  //!< 000..999
		bool m_hasTimezone;
		TimeZone m_timezone;

	public:
		DateTime()
		: m_year(-1),
		  m_month(-1),
		  m_day(-1),
		  m_hour(-1),
		  m_minute(-1),
		  m_second(-1),
		  m_millisecond(-1),
		  m_hasTimezone(false),
		  m_timezone()
		{
		}

		DateTime(int32_t y, int16_t mo, int16_t d, int16_t h, int16_t m, int16_t s, int16_t ms)
		: m_year(y),
		  m_month(mo),
		  m_day(d),
		  m_hour(h),
		  m_minute(m),
		  m_second(s),
		  m_millisecond(ms),
		  m_hasTimezone(false),
		  m_timezone()
		{
		}

		DateTime(int32_t y, int16_t mo, int16_t d, int16_t h, int16_t m, int16_t s, int16_t ms, const TimeZone & tz)
		: m_year(y),
		  m_month(mo),
		  m_day(d),
		  m_hour(h),
		  m_minute(m),
		  m_second(s),
		  m_millisecond(ms),
		  m_hasTimezone(true),
		  m_timezone(tz)
		{
		}

		bool hasYear() const
		{
			return m_year > 0;
		}

		bool hasMonth() const
		{
			return m_month > 0;
		}

		bool hasDay() const
		{
			return m_day > 0;
		}

		bool hasHour() const
		{
			return m_hour >= 0;
		}

		bool hasMinute() const
		{
			return m_minute >= 0;
		}

		bool hasSecond() const
		{
			return m_second >= 0;
		}

		bool hasMillisecond() const
		{
			return m_millisecond >= 0;
		}

		bool hasTimezone() const
		{
			return m_hasTimezone;
		}

		bool hasDate() const
		{
			return hasYear() && hasMonth() && hasDay();
		}

		bool hasTime() const
		{
			return hasHour() && hasMinute() && hasSecond();
		}

		bool isComplete() const
		{
			return hasDate() && hasTime() && hasMillisecond() && hasTimezone();
		}

		bool isEmpty() const
		{
			return !hasDate() && !hasTime();
		}

		EType getType() const
		{
			return (m_timezone.isUTC()) ? UTC : LOCAL;
		}

		int32_t getYear() const
		{
			return m_year;
		}

		int16_t getMonth() const
		{
			return m_month;
		}

		int16_t getDay() const
		{
			return m_day;
		}

		int16_t getHour() const
		{
			return m_hour;
		}

		int16_t getMinute() const
		{
			return m_minute;
		}

		int16_t getSecond() const
		{
			return m_second;
		}

		int16_t getMillisecond() const
		{
			return m_millisecond;
		}

		const TimeZone & getTimezone() const
		{
			return m_timezone;
		}

		std::string toString() const;
	};

	class UnixTime
	{
		int64_t m_seconds;   //!< 0..INT64_MAX
		int m_milliseconds;  //!< 0..999

	public:
		UnixTime(int64_t seconds = -1, int milliseconds = -1)
		: m_seconds(seconds),
		  m_milliseconds(milliseconds)
		{
		}

		bool isEmpty() const
		{
			return m_seconds < 0;
		}

		bool hasMilliseconds() const
		{
			return m_milliseconds >= 0;
		}

		int64_t getSeconds() const
		{
			return m_seconds;
		}

		int getMilliseconds() const
		{
			return m_milliseconds;
		}

		std::string toString() const;
	};
}
