/**
 * @file
 * @brief Data type flags.
 */

#pragma once

namespace ctp
{
	/**
	 * @brief Provides unique bit flag for each data type.
	 * It is used in client-server communication.
	 */
	namespace EDataFlags
	{
		enum
		{
			CONNECTION = (1 << 0)
		};
	}
}
