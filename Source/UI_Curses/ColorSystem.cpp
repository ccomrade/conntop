/**
 * @file
 * @brief Implementation of ColorSystem class.
 */

#include <curses.h>
#include <cstdlib>  // std::atexit
#include <new>

#include "ColorSystem.hpp"
#include "Exception.hpp"

ColorSystem *gColorSystem;

ColorSystem::ColorSystem()
{
	if (has_colors() == FALSE)
	{
		throw Exception("No color support", "UI_Curses");
	}

	if (start_color() == ERR)
	{
		throw Exception("Curses color start failed", "UI_Curses");
	}

	gLog->info("[UI_Curses] Number of colors supported by terminal: %d", COLORS);
	gLog->info("[UI_Curses] Number of supported color pairs: %d (required: %d)", COLOR_PAIRS, _COLOR_PAIR_MAX-1);

	if (_COLOR_PAIR_MAX >= COLOR_PAIRS)
	{
		throw Exception("Not enough color pairs", "UI_Curses");
	}

	short fgDefaultColor = COLOR_WHITE;
	short bgDefaultColor = COLOR_BLACK;

#ifdef NCURSES_VERSION  // ncurses-specific code
	use_default_colors();
	fgDefaultColor = -1;
	bgDefaultColor = -1;
#endif

	init_pair(RED_ON_DEFAULT, COLOR_RED, bgDefaultColor);
	init_pair(RED_ON_CYAN, COLOR_RED, COLOR_CYAN);
	init_pair(GREEN_ON_DEFAULT, COLOR_GREEN, bgDefaultColor);
	init_pair(GREEN_ON_CYAN, COLOR_GREEN, COLOR_CYAN);
	init_pair(YELLOW_ON_DEFAULT, COLOR_YELLOW, bgDefaultColor);
	init_pair(YELLOW_ON_CYAN, COLOR_YELLOW, COLOR_CYAN);
	init_pair(CYAN_ON_DEFAULT, COLOR_CYAN, bgDefaultColor);
	init_pair(BLACK_ON_CYAN, COLOR_BLACK, COLOR_CYAN);
	init_pair(BLACK_ON_GREEN, COLOR_BLACK, COLOR_GREEN);
	init_pair(DEFAULT_ON_CYAN, fgDefaultColor, COLOR_CYAN);

	gLog->info("[UI_Curses] Colors initialized");
}

void ColorSystem::Init()
{
	gColorSystem = new ColorSystem();

	auto DestroyGlobalColorSystem = []() -> void
	{
		delete gColorSystem;
		gColorSystem = nullptr;
	};

	std::atexit(DestroyGlobalColorSystem);
}

const int ColorSystem::ATTRIBUTE_TABLE[] = {
	[ATTR_DEFAULT] = A_NORMAL | COLOR_PAIR(DEFAULT),
	[ATTR_IMPORTANT_RED] = A_BOLD | COLOR_PAIR(RED_ON_DEFAULT),
	[ATTR_IMPORTANT_GREEN] = A_BOLD | COLOR_PAIR(GREEN_ON_DEFAULT),
	[ATTR_FUNCTION_KEY_BAR_LABEL] = COLOR_PAIR(DEFAULT),
	[ATTR_FUNCTION_KEY_BAR_NAME] = COLOR_PAIR(BLACK_ON_CYAN),
	[ATTR_STATUS_LABEL] = COLOR_PAIR(CYAN_ON_DEFAULT),
	[ATTR_STATUS_VALUE] = A_BOLD | COLOR_PAIR(CYAN_ON_DEFAULT),
	[ATTR_LIST_HEADER] = COLOR_PAIR(BLACK_ON_GREEN),
	[ATTR_SELECTED_ROW] = A_BOLD | COLOR_PAIR(DEFAULT_ON_CYAN),
	[ATTR_UNRESOLVED] = COLOR_PAIR(RED_ON_DEFAULT),
	[ATTR_UNRESOLVED_SELECTED] = COLOR_PAIR(RED_ON_CYAN),
	[ATTR_TRAFFIC] = A_BOLD | COLOR_PAIR(CYAN_ON_DEFAULT),
	[ATTR_TRAFFIC_SELECTED] = A_BOLD | COLOR_PAIR(DEFAULT_ON_CYAN),
	[ATTR_PROTO_UDP] = A_BOLD | COLOR_PAIR(YELLOW_ON_DEFAULT),
	[ATTR_PROTO_UDP_SELECTED] = A_BOLD | COLOR_PAIR(YELLOW_ON_CYAN),
	[ATTR_PROTO_TCP] = A_BOLD | COLOR_PAIR(GREEN_ON_DEFAULT),
	[ATTR_PROTO_TCP_SELECTED] = A_BOLD | COLOR_PAIR(GREEN_ON_CYAN),
	[ATTR_COUNTRY_CODE] = A_BOLD | COLOR_PAIR(YELLOW_ON_DEFAULT),
	[ATTR_COUNTRY_CODE_SELECTED] = A_BOLD | COLOR_PAIR(YELLOW_ON_CYAN)
};

const unsigned int ColorSystem::ATTRIBUTE_TABLE_SIZE = sizeof ATTRIBUTE_TABLE / sizeof ATTRIBUTE_TABLE[0];
