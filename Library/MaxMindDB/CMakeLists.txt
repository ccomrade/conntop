#
# conntop - MaxMindDB library
#

project(libmaxminddb VERSION 1.3.2 LANGUAGES C)

add_library(maxminddb STATIC
  Source/data-pool.c
  Source/maxminddb.c
)
add_library(conntop::MaxMindDB ALIAS maxminddb)

target_sources(maxminddb PRIVATE
  Source/data-pool.h
  Source/maxminddb-compat-util.h
  Include/maxminddb.h
)

target_include_directories(maxminddb PUBLIC
  Include
  ${CMAKE_CURRENT_BINARY_DIR}  # maxminddb_config.h
  ${CMAKE_BINARY_DIR}  # conntop_config.h
)

set_property(TARGET maxminddb PROPERTY C_STANDARD 99)
set_property(TARGET maxminddb PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET maxminddb PROPERTY C_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID STREQUAL GNU OR CMAKE_C_COMPILER_ID STREQUAL Clang)
	target_compile_options(maxminddb PRIVATE -Wall -Wextra)
endif()

if(TARGET Threads::Threads)
	target_link_libraries(maxminddb PUBLIC Threads::Threads)  # -pthread compiler flag
endif()

configure_file(maxminddb_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/maxminddb_config.h NEWLINE_STYLE LF)
