/**
 * @file
 * @brief PollHandle class.
 */

#pragma once

#include <poll.h>
#include <vector>

#include "Events.hpp"

namespace ctp
{
	struct PollEvent
	{
		static constexpr int ID = EGlobalEventID::POLL_SYSTEM_INTERNAL_EVENT;

	private:
		int m_fd;
		int m_flags;
		bool m_hasError;
		bool m_hasHangUp;
		bool m_hasInvalidFD;

	public:
		PollEvent( int fd = -1, int flags = 0, bool hasError = false, bool hasHangUp = false, bool hasInvalidFD = false )
		: m_fd(fd),
		  m_flags(flags),
		  m_hasError(hasError),
		  m_hasHangUp(hasHangUp),
		  m_hasInvalidFD(hasInvalidFD)
		{
		}

		bool isEmpty() const
		{
			return m_fd < 0;
		}

		int getDescriptor() const
		{
			return m_fd;
		}

		int getFlags() const
		{
			return m_flags;
		}

		bool hasError() const
		{
			return m_hasError;
		}

		bool hasHangUp() const
		{
			return m_hasHangUp;
		}

		bool hasInvalidFD() const
		{
			return m_hasInvalidFD;
		}
	};

	class PollHandle
	{
		std::vector<pollfd> m_descriptors;
		long m_eventIndex;

	public:
		PollHandle();
		~PollHandle();

		void add( int fd, int flags );
		void reset( int fd, int flags );
		void remove( int fd );

		void wait();

		PollEvent getNextEvent();
	};
}
