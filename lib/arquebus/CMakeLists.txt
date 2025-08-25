add_library(arquebus INTERFACE)

add_library(arquebus::arquebus ALIAS arquebus)

target_link_system_libraries(arquebus INTERFACE rt)

target_include_directories(arquebus INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${PROJECT_BINARY_DIR}/include/)

configure_file(version.hpp.in "${CMAKE_BINARY_DIR}/include/arquebus/version.hpp" ESCAPE_QUOTES)
