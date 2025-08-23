add_executable(consumer main.cpp)

target_link_libraries(consumer PRIVATE arquebus::arquebus_options arquebus::arquebus_warnings)

target_link_system_libraries(consumer PRIVATE arquebus::arquebus)
