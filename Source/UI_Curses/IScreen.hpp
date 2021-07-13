/**
 * @file
 * @brief IScreen class.
 */

#pragma once

namespace ctp
{
	struct IScreen
	{
		virtual ~IScreen() = default;

		virtual bool isRefreshRequired() = 0;
		virtual void refresh() = 0;
		virtual void invalidate() = 0;
		virtual void onResize() = 0;
		virtual bool onKey(int ch) = 0;
	};
}
