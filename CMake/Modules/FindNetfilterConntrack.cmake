#.rst:
# FindNetfilterConntrack
# ----------------------
#
# This module finds the libnetfilter_conntrack library.
#
# The following variables are set:
#
# ::
#
#   NETFILTER_CONNTRACK_FOUND
#   NETFILTER_CONNTRACK_INCLUDE_DIRS
#   NETFILTER_CONNTRACK_LIBRARIES
#
# The following target is created:
#
# ::
#
#   Netfilter::Conntrack
#

find_path(NETFILTER_CONNTRACK_INCLUDE_PATH NAMES libnetfilter_conntrack/libnetfilter_conntrack.h)

find_library(NETFILTER_CONNTRACK_LIBRARY NAMES netfilter_conntrack)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Netfilter_Conntrack
  REQUIRED_VARS NETFILTER_CONNTRACK_LIBRARY NETFILTER_CONNTRACK_INCLUDE_PATH
)

mark_as_advanced(NETFILTER_CONNTRACK_FOUND NETFILTER_CONNTRACK_LIBRARY NETFILTER_CONNTRACK_INCLUDE_PATH)

if(NETFILTER_CONNTRACK_FOUND)
	set(NETFILTER_CONNTRACK_INCLUDE_DIRS ${NETFILTER_CONNTRACK_INCLUDE_PATH})
	set(NETFILTER_CONNTRACK_LIBRARIES ${NETFILTER_CONNTRACK_LIBRARY})

	if(NOT TARGET Netfilter::Conntrack)
		add_library(Netfilter::Conntrack INTERFACE IMPORTED)
		set_target_properties(Netfilter::Conntrack PROPERTIES
		  INTERFACE_INCLUDE_DIRECTORIES "${NETFILTER_CONNTRACK_INCLUDE_PATH}"
		  INTERFACE_LINK_LIBRARIES "${NETFILTER_CONNTRACK_LIBRARY}"
		)
	endif()
endif()
