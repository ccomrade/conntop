#
# conntop - Netfilter collector
#

if(NOT CMAKE_SYSTEM_NAME STREQUAL Linux)
	message(STATUS "conntop: non-Linux system detected, Netfilter collector skipped")
	return()
endif()

find_package(NetfilterConntrack)

if(NOT NETFILTER_CONNTRACK_FOUND)
	message("conntop: libnetfilter_conntrack library not found, Netfilter collector skipped")
	return()
endif()

add_library(collector_netfilter STATIC
  CCollector.cpp
  Conntrack.cpp
)
add_library(conntop::Collector_Netfilter ALIAS collector_netfilter)

target_sources(collector_netfilter PRIVATE
  CCollector.hpp
  Conntrack.hpp
)

target_link_libraries(collector_netfilter PUBLIC
  conntop::Base
  Netfilter::Conntrack
)
