cmake_minimum_required(VERSION 3.20)
project(conntop)

set(CMAKE_CXX_STANDARD 17)
include_directories(Code)

add_compile_options(
	-Wall -Wextra -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion
	-Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference  -Wuseless-cast
	-Wdouble-promotion -Wformat=2
	-pedantic
)

add_executable(conntop
	Code/App/App.cpp
	Code/App/App.h
	Code/App/Connection.h
	Code/App/LocalSocket.h
	Code/App/LocalSocketCollection.h
	Code/App/Probe/Conntrack.cpp
	Code/App/Probe/Conntrack.h
	Code/App/Probe/Probe.cpp
	Code/App/Probe/Probe.h
	Code/App/Probe/SocketPids.cpp
	Code/App/Probe/SocketPids.h
	Code/Base/Base64.cpp
	Code/Base/Base64.h
	Code/Base/Endian.h
	Code/Base/Format.cpp
	Code/Base/Format.h
	Code/Base/HTTP.h
	Code/Base/HTTPMethod.h
	Code/Base/HTTPStatus.h
	Code/Base/IPAddress.cpp
	Code/Base/IPAddress.h
	Code/Base/IPProtocol.h
	Code/System/CmdLine.cpp
	Code/System/CmdLine.h
	Code/System/Handle.h
	Code/System/Log.cpp
	Code/System/Log.h
	Code/System/Main.cpp
	Code/System/Reactor.cpp
	Code/System/Reactor.h
	Code/System/System.cpp
	Code/System/System.h
	Code/System/TCPServerSocket.cpp
	Code/System/TCPServerSocket.h
	Code/System/TCPSocket.cpp
	Code/System/TCPSocket.h
)

target_link_libraries(conntop PUBLIC netfilter_conntrack)
