/**
 * @file
 * @brief ICollector class.
 */

#pragma once

#include "KString.hpp"

namespace ctp
{
	// Connection.hpp
	struct IConnectionUpdateCallback;

	struct ICollector
	{
		virtual ~ICollector() = default;

		virtual KString getName() const = 0;

		virtual void init(IConnectionUpdateCallback *callback) = 0;

		virtual void onUpdate() = 0;

		virtual bool isPaused() const = 0;
		virtual void setPaused(bool paused) = 0;
	};
}
