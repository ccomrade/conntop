/**
 * @file
 * @brief PollSystem class for Unix platform.
 */

#pragma once

#include <memory>
#include <functional>

namespace ctp
{
	namespace EPollFlags
	{
		enum
		{
			INPUT  = (1 << 0),
			OUTPUT = (1 << 1),

			ERROR  = (1 << 2)
		};
	};

	class PollSystem
	{
	public:
		using Callback = std::function<void(int, void*)>;

	private:
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		PollSystem();
		~PollSystem();

		template<class T>
		void add( T & object, int flags, const Callback & callback, void *param )
		{
			return addFD( object.getFD(), flags, callback, param );
		}

		template<class T>
		void reset( T & object, int flags )
		{
			return resetFD( object.getFD(), flags );
		}

		template<class T>
		void remove( T & object )
		{
			return removeFD( object.getFD() );
		}

		// Platform-specific functions

		void addFD( int fd, int flags, const Callback & callback, void *param );
		void resetFD( int fd, int flags );
		void removeFD( int fd );
	};
}
