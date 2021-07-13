/**
 * @file
 * @brief Platform class for Unix platform.
 */

#pragma once

#include <string>
#include <memory>

#include "GlobalEnvironment.hpp"
#include "KString.hpp"
#include "DateTime.hpp"

namespace ctp
{
	class PlatformProcessMemoryUsage
	{
		long m_totalKBytes;
		long m_peakTotalKBytes;
		long m_anonymousKBytes;
		long m_mappedFilesKBytes;
		long m_sharedKBytes;

	public:
		PlatformProcessMemoryUsage(long total, long peakTotal, long anonymous, long mappedFiles, long shared)
		: m_totalKBytes(total),
		  m_peakTotalKBytes(peakTotal),
		  m_anonymousKBytes(anonymous),
		  m_mappedFilesKBytes(mappedFiles),
		  m_sharedKBytes(shared)
		{
		}

		bool hasTotalSize() const
		{
			return m_totalKBytes >= 0;
		}

		bool hasPeakTotalSize() const
		{
			return m_peakTotalKBytes >= 0;
		}

		bool hasAnonymousSize() const
		{
			return m_anonymousKBytes >= 0;
		}

		bool hasMappedFilesSize() const
		{
			return m_mappedFilesKBytes >= 0;
		}

		bool hasSharedSize() const
		{
			return m_sharedKBytes >= 0;
		}

		long getTotalSize() const
		{
			return m_totalKBytes;
		}

		long getPeakTotalSize() const
		{
			return m_peakTotalKBytes;
		}

		long getAnonymousSize() const
		{
			return m_anonymousKBytes;
		}

		long getMappedFilesSize() const
		{
			return m_mappedFilesKBytes;
		}

		long getSharedSize() const
		{
			return m_sharedKBytes;
		}
	};

	class Platform
	{
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		Platform();
		~Platform();

		KString getImplementationName() const;
		KString getSystemName() const;

		void start();
		void stop();

		std::string getCurrentHostName();

		UnixTime getCurrentUnixTime();
		DateTime getCurrentDateTime(DateTime::EType type = DateTime::LOCAL);

		PlatformProcessMemoryUsage getProcessMemoryUsage();
	};
}
