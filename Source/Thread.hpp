/**
 * @file
 * @brief Thread class.
 */

#pragma once

#include <string>
#include <thread>
#include <functional>

#include "KString.hpp"

namespace ctp
{
	class Thread
	{
	public:
		using Function = std::function<void(void)>;

		static const KString DEFAULT_NAME;

	private:
		std::string m_name;
		std::thread m_thread;

		static void Run(std::string threadName, Function threadFunction);

		static void PlatformCurrentThreadSetName(const KString & name);

	public:
		Thread()
		: m_name(),
		  m_thread()
		{
		}

		Thread(std::string threadName, const Function & threadFunction);

		// no copy
		Thread(const Thread &) = delete;
		Thread & operator=(const Thread &) = delete;

		// move allowed
		Thread(Thread &&) = default;
		Thread & operator=(Thread &&) = default;

		const std::string & getName() const
		{
			return m_name;
		}

		bool isJoinable() const
		{
			return m_thread.joinable();
		}

		std::thread::native_handle_type getNativeHandle()
		{
			return m_thread.native_handle();
		}

		void join();

		static KString GetCurrentThreadName();
		static void SetCurrentThreadName(std::string name);
	};
}
