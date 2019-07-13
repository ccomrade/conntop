#
# conntop - ConcurrentQueue library
#

add_library(concurrentqueue INTERFACE)
add_library(conntop::ConcurrentQueue ALIAS concurrentqueue)

target_include_directories(concurrentqueue INTERFACE
  Include
)
