#
# conntop - Curses UI
#

if(NOT UNIX)
	message(STATUS "conntop: non-Unix system detected, Curses UI skipped")
	return()
endif()

find_package(Curses)

if(NOT CURSES_FOUND)
	message("conntop: curses library not found, Curses UI skipped")
	return()
endif()

add_library(ui_curses STATIC
  ColorSystem.cpp
  CUI.cpp
  Curses.cpp
  FunctionKeyBar.cpp
  Screen.cpp
  Screens/DialogConnectionColumns.cpp
  Screens/DialogConnectionDetails.cpp
  Screens/DialogConnectionSort.cpp
  Screens/ScreenConnectionList.cpp
  Screens/ScreenHelp.cpp
)
add_library(conntop::UI_Curses ALIAS ui_curses)

target_sources(ui_curses PRIVATE
  ColorSystem.hpp
  CUI.hpp
  Curses.hpp
  CursesEvent.hpp
  FunctionKeyBar.hpp
  IScreen.hpp
  Screen.hpp
  Screens/DialogConnectionColumns.hpp
  Screens/DialogConnectionDetails.hpp
  Screens/DialogConnectionSort.hpp
  Screens/ScreenConnectionList.hpp
  Screens/ScreenHelp.hpp
)

target_include_directories(ui_curses PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(ui_curses PUBLIC
  conntop::Base
  ${CURSES_LIBRARIES}
)
