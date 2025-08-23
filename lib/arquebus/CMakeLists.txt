include(GenerateExportHeader)

add_library(arquebus api.cpp)

add_library(arquebus::arquebus ALIAS arquebus)

target_link_libraries(arquebus PRIVATE arquebus::arquebus_options arquebus::arquebus_warnings)

target_link_system_libraries(arquebus PRIVATE fmt::fmt)

target_include_directories(
  arquebus
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/private>
)

# Ensure that we have default symbol visibility of hidden
set_target_properties(
  arquebus
  PROPERTIES VERSION ${PROJECT_VERSION}
             CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
)

generate_export_header(arquebus EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/arquebus/arquebus_export.hpp")
configure_file(include/public/arquebus/version.hpp.in "${CMAKE_BINARY_DIR}/include/arquebus/version.hpp" ESCAPE_QUOTES)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(arquebus PUBLIC EXAMPLE_STATIC_DEFINE)
endif()
