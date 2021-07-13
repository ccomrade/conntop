/**
 * @file
 * @brief Implementation of platform-specific functions from Thread class for Unix platform.
 */

#include "Thread.hpp"
#include "conntop_config.h"

#ifdef CONNTOP_PLATFORM_LINUX
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace ctp
{
	void Thread::PlatformCurrentThreadSetName(const KString & name)
	{
		if (name.empty())
			return;

	#ifdef CONNTOP_PLATFORM_LINUX
		// this function returns true if current thread is main thread
		auto IsMainThread = []() -> bool
		{
			const pid_t processID = getpid();
			const pid_t threadID = syscall(SYS_gettid);

			return processID == threadID;
		};

		// do not change name of main thread to prevent process rename
		if (!IsMainThread())
		{
			prctl(PR_SET_NAME, name.c_str());
		}
	#endif
	}
}
