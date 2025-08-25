cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

# std C++26
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(
  arquebus
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "An experiment in low latency inter-process shared memory queues"
)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/GitHash.cmake)
include(ProjectOptions.cmake)

arquebus_setup_options()

include(Dependencies.cmake)
arquebus_setup_dependencies()

arquebus_local_options()

# need to add Catch2 extras folder to module path so we can include(Catch)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

target_compile_features(arquebus_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(arquebus::arquebus_options ALIAS arquebus_options)
add_library(arquebus::arquebus_warnings ALIAS arquebus_warnings)

add_subdirectory(lib)
add_subdirectory(src)

include(cmake/TestSetupHelpers.cmake)

add_subdirectory(test)

# create a custom target to copy the compile_commands.json from the build into the root level for external tools using
# clangd add_custom_target( copy_compile_commands ALL COMMENT "copy compile_commands.json to root level" COMMAND echo
# "--- Copy compile_commands.json to root level..." COMMAND ${CMAKE_COMMAND} -E copy
# ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR} )
