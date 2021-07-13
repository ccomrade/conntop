/**
 * @file
 * @brief Implementation of classes for storing date and time information.
 */

#include "DateTime.hpp"
#include "CharBuffer.hpp"

std::string DateTime::toString() const
{
	CharBuffer<256> buffer;

	if (hasDate())
	{
		// date format
		buffer.append_f("%.4d-%.2d-%.2d", m_year, m_month, m_day);
	}

	if (hasTime())
	{
		if (hasDate())
		{
			// space between date and time
			buffer.append(' ');
		}

		// time format
		buffer.append_f("%.2d:%.2d:%.2d", m_hour, m_minute, m_second);

		if (hasMillisecond())
		{
			buffer.append_f(".%.3d", m_millisecond);
		}

		if (hasTimezone())
		{
			if (m_timezone.isUTC())
			{
				buffer.append('Z');
			}
			else
			{
				char tzSign = (m_timezone.isNegative()) ? '-' : '+';
				int tzHour = m_timezone.getHourOffset();
				int tzMinute = m_timezone.getMinuteOffset();

				buffer.append_f("%c%.2d%.2d", tzSign, tzHour, tzMinute);
			}
		}
	}

	return buffer.toString();
}

std::string UnixTime::toString() const
{
	CharBuffer<32> buffer;

	if (!isEmpty())
	{
		if (hasMilliseconds())
		{
			buffer.append_f("%zd.%.3d", m_seconds, m_milliseconds);
		}
		else
		{
			buffer.append_f("%zd", m_seconds);
		}
	}

	return buffer.toString();
}
