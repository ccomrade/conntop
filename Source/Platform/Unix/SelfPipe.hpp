/**
 * @file
 * @brief SelfPipe class.
 */

#pragma once

#include "Types.hpp"

namespace ctp
{
	class SelfPipe
	{
		int m_pipefd[2];

		void destroy();

	public:
		SelfPipe();

		~SelfPipe()
		{
			destroy();
		}

		int getFD()
		{
			return getReadFD();
		}

		int getReadFD()
		{
			return m_pipefd[0];
		}

		int getWriteFD()
		{
			return m_pipefd[1];
		}

		size_t writeData(const char *data, size_t dataLength);

		size_t readData(char *buffer, size_t bufferSize);

		size_t clear();
	};
}
