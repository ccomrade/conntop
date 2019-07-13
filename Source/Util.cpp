/**
 * @file
 * @brief Implementation of utilities.
 */

#include <cstdio>  // std::snprintf

#include "Util.hpp"
#include "Platform.hpp"
#include "Log.hpp"
#include "Address.hpp"
#include "Port.hpp"

namespace ctp
{
	static constexpr char SIZE_UNITS[] = { 'k', 'M', 'G', 'T', 'P', 'E', 'Z' };

	/**
	 * @brief Converts size in bytes to human readable form.
	 * @param bytes The size in bytes.
	 * @return String with human readable size.
	 */
	std::string Util::GetHumanReadableSize( uint64_t bytes )
	{
		constexpr unsigned int LAST_UNIT = (sizeof SIZE_UNITS) - 1;

		if ( bytes == 0 )
		{
			return std::string( "0 B" );
		}

		if ( bytes < 2000 )
		{
			std::string result = std::to_string( bytes );
			result += " B";
			return result;
		}

		double value = bytes;
		unsigned int unit = 0;
		while ( unit < LAST_UNIT )
		{
			value /= 1000;
			if ( value < 2000 )
			{
				break;
			}
			unit++;
		}

		char buffer[16];
		if ( std::snprintf( buffer, sizeof buffer, "%.1f %cB", value, SIZE_UNITS[unit] ) < 0 )
		{
			return std::string();
		}

		return std::string( buffer );
	}

	/**
	 * @brief Creates "address:port" string.
	 * @param address The address.
	 * @param portNumber The port.
	 * @return String with "address:port".
	 */
	std::string Util::AddressPortToString( const IAddress & address, uint16_t portNumber )
	{
		std::string result;

		if ( address.getType() == EAddressType::IP6 )
		{
			result += '[';
			result += address.toString();
			result += ']';
		}
		else
		{
			result += address.toString();
		}

		result += ':';
		result += std::to_string( portNumber );

		return result;
	}

	/**
	 * @brief Creates "address:port" string.
	 * @param address The address.
	 * @param port The port.
	 * @return String with "address:port".
	 */
	std::string Util::AddressPortToString( const IAddress & address, const Port & port )
	{
		return AddressPortToString( address, port.getNumber() );
	}

	/**
	 * @brief Dumps memory usage information to log.
	 * @param always If true, always log message is used instead of info message.
	 */
	void Util::LogMemoryUsage( bool always )
	{
		if ( ! gLog || ! gPlatform )
		{
			return;
		}

		const Log::EType msgType = (always) ? Log::ALWAYS : Log::INFO;

		if ( ! gLog->isMsgEnabled( msgType ) )
		{
			return;
		}

		const PlatformProcessMemoryUsage memUsage = gPlatform->getProcessMemoryUsage();

		auto LogUsageRow = [msgType]( const char *label, long value )
		{
			if ( value < 0 )
				gLog->log( msgType, "%s           N/A", label );
			else
				gLog->log( msgType, "%s %10ld kB", label, value );
		};

		gLog->log( msgType, "-------- MEMORY USAGE --------" );

		LogUsageRow( "Shared:         ", memUsage.getSharedSize() );
		LogUsageRow( "Anonymous:      ", memUsage.getAnonymousSize() );
		LogUsageRow( "Mapped files:   ", memUsage.getMappedFilesSize() );
		LogUsageRow( "--> Total:      ", memUsage.getTotalSize() );
		LogUsageRow( "--> Peak total: ", memUsage.getPeakTotalSize() );
	}
}
