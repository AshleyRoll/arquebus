add_executable(producer main.cpp)

target_link_libraries(producer PRIVATE arquebus::arquebus_options arquebus::arquebus_warnings fmt::fmt)

target_link_system_libraries(producer PRIVATE arquebus::arquebus)
