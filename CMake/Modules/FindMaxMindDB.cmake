#.rst:
# FindMaxMindDB
# -------------
#
# This module finds the libmaxminddb library.
#
# The following variables are set:
#
# ::
#
#   MAXMINDDB_FOUND
#   MAXMINDDB_INCLUDE_DIRS
#   MAXMINDDB_LIBRARIES
#
# The following target is created:
#
# ::
#
#   MaxMindDB::MaxMindDB
#

find_path(MAXMINDDB_INCLUDE_PATH NAMES maxminddb.h)

find_library(MAXMINDDB_LIBRARY NAMES maxminddb)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MaxMindDB
  REQUIRED_VARS MAXMINDDB_LIBRARY MAXMINDDB_INCLUDE_PATH
)

mark_as_advanced(MAXMINDDB_FOUND MAXMINDDB_LIBRARY MAXMINDDB_INCLUDE_PATH)

if(MAXMINDDB_FOUND)
	set(MAXMINDDB_INCLUDE_DIRS ${MAXMINDDB_INCLUDE_PATH})
	set(MAXMINDDB_LIBRARIES ${MAXMINDDB_LIBRARY})

	if(NOT TARGET MaxMindDB::MaxMindDB)
		add_library(MaxMindDB::MaxMindDB INTERFACE IMPORTED)
		set_target_properties(MaxMindDB::MaxMindDB PROPERTIES
		  INTERFACE_INCLUDE_DIRECTORIES "${MAXMINDDB_INCLUDE_PATH}"
		  INTERFACE_LINK_LIBRARIES "${MAXMINDDB_LIBRARY}"
		)
	endif()
endif()
