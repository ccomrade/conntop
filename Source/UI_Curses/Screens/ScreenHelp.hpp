/**
 * @file
 * @brief ScreenHelp class.
 */

#pragma once

#include "Screen.hpp"

class ScreenHelp : public Screen
{
	void draw();

	void handleResize() override;
	bool handleKey(int ch) override;

public:
	ScreenHelp();
};
