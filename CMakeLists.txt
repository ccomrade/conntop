#
# conntop project
#

cmake_minimum_required(VERSION 3.2)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Type of build")
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

project(conntop VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/Modules)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(conntop_base INTERFACE)
add_library(conntop::Base ALIAS conntop_base)

target_include_directories(conntop_base INTERFACE
  Source
  ${CMAKE_BINARY_DIR}  # conntop_config.h
)

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	target_compile_options(conntop_base INTERFACE -Wall -Wextra)
endif()

if(UNIX)
	set(CONNTOP_PLATFORM_UNIX TRUE)
	if(CMAKE_SYSTEM_NAME STREQUAL Linux)
		set(CONNTOP_PLATFORM_LINUX TRUE)
	endif()
	set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
	set(THREADS_PREFER_PTHREAD_FLAG TRUE)
endif()

find_package(Threads REQUIRED)

# conntop::ConcurrentQueue
add_subdirectory(Library/ConcurrentQueue)

# conntop::ReaderWriterQueue
add_subdirectory(Library/ReaderWriterQueue)

# conntop::RapidJSON
add_subdirectory(Library/RapidJSON)

target_link_libraries(conntop_base INTERFACE
  Threads::Threads
  conntop::ConcurrentQueue
  conntop::ReaderWriterQueue
  conntop::RapidJSON
)

option(CONNTOP_DEDICATED "Build conntop dedicated server (conntopd)." OFF)

if(NOT CONNTOP_DEDICATED)
	option(CONNTOP_USE_OWN_LIBMAXMINDDB "Use own MaxMindDB library instead of system one." OFF)
	if(CONNTOP_USE_OWN_LIBMAXMINDDB)
		# conntop::MaxMindDB
		add_subdirectory(Library/MaxMindDB)
	endif()
endif()

if(CONNTOP_DEDICATED)
	set(CONNTOP_APP conntopd)
else()
	set(CONNTOP_APP conntop)
endif()

add_subdirectory(Source)

if(TARGET conntop::Collector_Netfilter)
	set(CONNTOP_COLLECTOR_NETFILTER TRUE)
endif()

if(TARGET conntop::UI_Curses)
	set(CONNTOP_UI_CURSES TRUE)
endif()

# create conntop_config.h in build directory
configure_file(conntop_config.h.in ${CMAKE_BINARY_DIR}/conntop_config.h NEWLINE_STYLE LF)
