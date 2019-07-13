#
# conntop - ReaderWriterQueue library
#

add_library(readerwriterqueue INTERFACE)
add_library(conntop::ReaderWriterQueue ALIAS readerwriterqueue)

target_include_directories(readerwriterqueue INTERFACE
  Include
)
