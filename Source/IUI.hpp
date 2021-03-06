/**
 * @file
 * @brief IUI class.
 */

#pragma once

#include "KString.hpp"

struct IUI
{
	virtual ~IUI() = default;

	virtual KString getName() const = 0;

	virtual void init() = 0;

	virtual void refreshConnectionList() = 0;
};
