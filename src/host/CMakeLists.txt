add_executable(host main.cpp)

target_link_libraries(host PRIVATE arquebus::arquebus_options arquebus::arquebus_warnings)

target_link_system_libraries(host PRIVATE arquebus::arquebus)
