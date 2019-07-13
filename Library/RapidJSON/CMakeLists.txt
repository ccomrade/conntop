#
# conntop - RapidJSON library
#

add_library(rapidjson INTERFACE)
add_library(conntop::RapidJSON ALIAS rapidjson)

target_include_directories(rapidjson INTERFACE
  Include
)
