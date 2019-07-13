#.rst:
# FindUnixRT
# ----------
#
# This module finds the POSIX.1b Realtime Extensions library (part of libc).
#
# The following variables are set:
#
# ::
#
#   UNIXRT_FOUND
#   UNIXRT_LIBRARIES
#
# The following target is created:
#
# ::
#
#   UnixRT::UnixRT
#

find_library(UNIXRT_LIBRARY NAMES rt)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UnixRT
  REQUIRED_VARS UNIXRT_LIBRARY
)

mark_as_advanced(UNIXRT_FOUND UNIXRT_LIBRARY)

if(UNIXRT_FOUND)
	set(UNIXRT_LIBRARIES ${UNIXRT_LIBRARY})

	if(NOT TARGET UnixRT::UnixRT)
		add_library(UnixRT::UnixRT INTERFACE IMPORTED)
		set_target_properties(UnixRT::UnixRT PROPERTIES
		  INTERFACE_LINK_LIBRARIES "${UNIXRT_LIBRARY}"
		)
	endif()
endif()
