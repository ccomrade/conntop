#
# conntop - Unix platform
#

find_package(UnixRT REQUIRED)

add_library(platform_unix STATIC
  Address.cpp
  Log.cpp
  Main.cpp
  Platform.cpp
  PollHandle.cpp
  PollSystem.cpp
  SelfPipe.cpp
  Sockets.cpp
  Thread.cpp
  Util.cpp
)
add_library(conntop::Platform ALIAS platform_unix)

target_sources(platform_unix PRIVATE
  GetAddrInfo.hpp
  Log.hpp
  Platform.hpp
  PollHandle.hpp
  PollSystem.hpp
  SelfPipe.hpp
  Sockets.hpp
)

if(NOT CONNTOP_DEDICATED)
	target_sources(platform_unix PRIVATE
	  GeoIP.cpp
	  Resolver.cpp
	)
endif()

target_link_libraries(platform_unix PUBLIC
  conntop::Base
  UnixRT::UnixRT
)
