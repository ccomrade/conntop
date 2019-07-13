#
# conntop
#

if(UNIX)
	add_subdirectory(Platform/Unix)
endif()

if(NOT TARGET conntop::Platform)
	message(FATAL_ERROR "conntop: Unsupported platform")
endif()

# conntop::Collector_Netfilter
add_subdirectory(Collector_Netfilter)

if(NOT CONNTOP_DEDICATED)
	# conntop::UI_Curses
	add_subdirectory(UI_Curses)
endif()

add_executable(${CONNTOP_APP}
  App.cpp
  ClientServerProtocol.cpp
  CmdLine.cpp
  CmdLineOptions.cpp
  Connection.cpp
  DateTime.cpp
  Events.cpp
  EventSystem.cpp
  GlobalEnvironment.cpp
  Server.cpp
  Thread.cpp
  Util.cpp
)

target_sources(${CONNTOP_APP} PRIVATE
  Address.hpp
  App.hpp
  CharBuffer.hpp
  ClientServerProtocol.hpp
  CmdLine.hpp
  Compiler.hpp
  Connection.hpp
  ConnectionStorage.hpp
  DataFlags.hpp
  DateTime.hpp
  Events.hpp
  EventSystem.hpp
  EventWrapper.hpp
  Exception.hpp
  GlobalEnvironment.hpp
  Hash.hpp
  ICollector.hpp
  IEventCallback.hpp
  KString.hpp
  Log.hpp
  Platform.hpp
  PollSystem.hpp
  Port.hpp
  Server.hpp
  SocketReaderWriter.hpp
  Sockets.hpp
  Thread.hpp
  Types.hpp
  Util.hpp
  Version.hpp
)

if(NOT CONNTOP_DEDICATED)
	target_sources(${CONNTOP_APP} PRIVATE
	  Client.cpp
	  ConnectionList.cpp
	  Country.cpp
	  GeoIP.cpp
	  Resolver.cpp
	  WhoisData.cpp
	)

	target_sources(${CONNTOP_APP} PRIVATE
	  ASN.hpp
	  Client.hpp
	  ClientEvent.hpp
	  ConnectionList.hpp
	  Country.hpp
	  GeoIP.hpp
	  IUI.hpp
	  Resolver.hpp
	  WhoisData.hpp
	)
endif()

target_link_libraries(${CONNTOP_APP} PRIVATE
  conntop::Base
  conntop::Platform
)

if(NOT CONNTOP_DEDICATED)
	# GeoIP library
	if(TARGET conntop::MaxMindDB)
		target_link_libraries(${CONNTOP_APP} PRIVATE conntop::MaxMindDB)
	else()
		find_package(MaxMindDB REQUIRED)
		target_link_libraries(${CONNTOP_APP} PRIVATE MaxMindDB::MaxMindDB)
	endif()
endif()

# Collector
if(TARGET conntop::Collector_Netfilter)
	target_link_libraries(${CONNTOP_APP} PRIVATE conntop::Collector_Netfilter)
	message(STATUS "conntop: Using Netfilter collector")
else()
	message(FATAL_ERROR "conntop: No collector implementation available")
endif()

if(NOT CONNTOP_DEDICATED)
	# UI
	if(TARGET conntop::UI_Curses)
		target_link_libraries(${CONNTOP_APP} PRIVATE conntop::UI_Curses)
		message(STATUS "conntop: Using Curses UI")
	else()
		message(FATAL_ERROR "conntop: No UI implementation available")
	endif()
endif()
