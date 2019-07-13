/**
 * @file
 * @brief Implementation of PollHandle class.
 */

#include <climits>  // INT_MIN
#include <cerrno>
#include <system_error>

#include "PollHandle.hpp"
#include "PollSystem.hpp"  // EPollFlags

namespace ctp
{
	static short FlagsToEvents( int flags )
	{
		short events = 0;

		if ( flags & EPollFlags::INPUT )
			events |= POLLIN;

		if ( flags & EPollFlags::OUTPUT )
			events |= POLLOUT;

		return events;
	}

	static int EventsToFlags( short events )
	{
		int flags = 0;

		if ( events & POLLIN )
			flags |= EPollFlags::INPUT;

		if ( events & POLLOUT )
			flags |= EPollFlags::OUTPUT;

		if ( events & POLLERR || events & POLLHUP || events & POLLNVAL )
			flags |= EPollFlags::ERROR;

		return flags;
	}

	PollHandle::PollHandle()
	: m_descriptors(),
	  m_eventIndex(-1)
	{
	}

	PollHandle::~PollHandle()
	{
	}

	void PollHandle::add( int fd, int flags )
	{
		pollfd descriptor;
		descriptor.fd = fd;
		descriptor.events = FlagsToEvents( flags );
		descriptor.revents = 0;

		m_descriptors.push_back( descriptor );
	}

	void PollHandle::reset( int fd, int flags )
	{
		const int negativeFD = (fd == 0) ? INT_MIN : -fd;

		for ( auto it = m_descriptors.begin(); it != m_descriptors.end(); ++it )
		{
			if ( it->fd == fd || it->fd == negativeFD )
			{
				it->fd = fd;
				it->events = FlagsToEvents( flags );
				break;
			}
		}
	}

	void PollHandle::remove( int fd )
	{
		const int negativeFD = (fd == 0) ? INT_MIN : -fd;

		for ( auto it = m_descriptors.begin(); it != m_descriptors.end(); )
		{
			if ( it->fd == fd || it->fd == negativeFD )
			{
				it = m_descriptors.erase( it );
			}
			else
			{
				++it;
			}
		}
	}

	void PollHandle::wait()
	{
		int status = poll( m_descriptors.data(), m_descriptors.size(), -1 );
		if ( status > 0 )
		{
			m_eventIndex = 0;
		}
		else if ( status < 0 && errno != EINTR )
		{
			throw std::system_error( errno, std::system_category(), "Poll failed" );
		}
		else
		{
			m_eventIndex = -1;
		}
	}

	PollEvent PollHandle::getNextEvent()
	{
		if ( m_eventIndex >= 0 )
		{
			for ( unsigned int i = m_eventIndex; i < m_descriptors.size(); i++ )
			{
				pollfd & descriptor = m_descriptors[i];

				if ( descriptor.revents != 0 )
				{
					m_eventIndex = i + 1;

					const int fd = descriptor.fd;
					const int events = descriptor.revents;
					const int flags = EventsToFlags( events );

					const bool hasError = events & POLLERR;
					const bool hasHangUp = events & POLLHUP;
					const bool hasInvalidFD = events & POLLNVAL;

					// ignore this file descriptor until reset is called
					descriptor.fd = (fd == 0) ? INT_MIN : -fd;

					return PollEvent( fd, flags, hasError, hasHangUp, hasInvalidFD );
				}
			}

			m_eventIndex = -1;
		}

		return PollEvent();
	}
}
