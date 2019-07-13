/**
 * @file
 * @brief IEventCallback class.
 */

#pragma once

namespace ctp
{
	template<class T>
	struct IEventCallback
	{
		virtual void onEvent( const T & event ) = 0;
	};
}
