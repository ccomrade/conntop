/**
 * @file
 * @brief Version information.
 */

#pragma once

#include "KString.hpp"
#include "conntop_config.h"

#ifdef NDEBUG
#define BUILD_NAME "release"
#else
#define BUILD_NAME "debug"
#endif

constexpr KString VERSION = CONNTOP_APP_NAME " " CONNTOP_VERSION_STRING "-" BUILD_NAME;

constexpr KString COPYRIGHT_NOTICE = "Copyright (C) 2019 Daniel Hryzbil";

constexpr KString LICENSE_NOTICE = "This program is free software: you can redistribute it and/or modify\n"
                                   "it under the terms of the GNU General Public License as published by\n"
                                   "the Free Software Foundation, either version 3 of the License, or\n"
                                   "(at your option) any later version.\n"
                                   "\n"
                                   "This program is distributed in the hope that it will be useful,\n"
                                   "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                                   "GNU General Public License for more details.\n"
                                   "\n"
                                   "You should have received a copy of the GNU General Public License\n"
                                   "along with this program.  If not, see <https://www.gnu.org/licenses/>.";
