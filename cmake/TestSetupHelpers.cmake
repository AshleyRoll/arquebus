# Create a Catch2 test with the provided TEST_SOURCES. The target will be ${TARGET_PREFIX}_tests and will privately link
# LINK_LIBRARIES private header path PRIVATE_HEADER_PATH will be added to allow testing of private headers
function(arquebus_catch2_test_setup)
  set(options)
  set(oneValueArgs TARGET_PREFIX)
  set(multiValueArgs TEST_SOURCES LINK_LIBRARIES PRIVATE_HEADER_PATH)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

  if("${ARG_TARGET_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "TARGET_PREFIX is mandatory")
  endif()

  if("${ARG_TEST_SOURCES}" STREQUAL "")
    message(FATAL_ERROR "No TEST_SOURCES provided")
  endif()

  set(tests_target ${ARG_TARGET_PREFIX}_tests)

  add_executable(${tests_target} ${ARG_TEST_SOURCES})
  target_link_libraries(${tests_target} PRIVATE ${ARG_LINK_LIBRARIES} Catch2::Catch2WithMain)

  if(NOT "${ARG_PRIVATE_HEADER_PATH}" STREQUAL "")
    target_include_directories(
      ${tests_target} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}${ARG_PRIVATE_HEADER_PATH}>
    )
  endif()

  if(TARGET ${tests_target})
    catch_discover_tests(
      ${tests_target}
      TEST_PREFIX
      "[${ARG_TARGET_PREFIX}:unit]-"
      REPORTER
      XML
      OUTPUT_DIR
      .
      OUTPUT_PREFIX
      "unitests."
      OUTPUT_SUFFIX
      .xml
    )
  endif()
endfunction()

# Create a Catch2 test with the provided CONSTEXPR_SOURCES. This will create two targets -
# ${TARGET_PREFIX}_constexpr_tests that runs at compile time - ${TARGET_PREFIX}_relaxed_constexpr_tests disables compile
# to to assist with debugging constexpr tests LINK_LIBRARIES will be privately linked PRIVATE_HEADER_PATH will be added
# to allow constexpr testing of private headers
function(arquebus_catch2_constexpr_test_setup)
  set(options)
  set(oneValueArgs TARGET_PREFIX PRIVATE_HEADER_PATH)
  set(multiValueArgs CONSTEXPR_SOURCES LINK_LIBRARIES)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

  if("${ARG_TARGET_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "TARGET_PREFIX is mandatory")
  endif()

  if("${ARG_CONSTEXPR_SOURCES}" STREQUAL "")
    message(FATAL_ERROR "No CONSTEXPR_SOURCES provided")
  endif()

  set(constexpr_target ${ARG_TARGET_PREFIX}_constexpr_tests)
  set(relaxed_constexpr_target ${ARG_TARGET_PREFIX}_relaxed_constexpr_tests)

  add_executable(${constexpr_target} ${ARG_CONSTEXPR_SOURCES})

  target_link_libraries(${constexpr_target} PRIVATE ${ARG_LINK_LIBRARIES} Catch2::Catch2WithMain)

  if(NOT "${ARG_PRIVATE_HEADER_PATH}" STREQUAL "")
    target_include_directories(
      ${constexpr_target} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}${ARG_PRIVATE_HEADER_PATH}>
    )
  endif()

  # Disable the constexpr portion of the test, this allows us to have an executable that we can debug when things go
  # wrong with the constexpr testing
  add_executable(${relaxed_constexpr_target} ${ARG_CONSTEXPR_SOURCES})

  target_link_libraries(${relaxed_constexpr_target} PRIVATE ${ARG_LINK_LIBRARIES} Catch2::Catch2WithMain)

  target_compile_definitions(${relaxed_constexpr_target} PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

  if(NOT "${ARG_PRIVATE_HEADER_PATH}" STREQUAL "")
    target_include_directories(
      ${relaxed_constexpr_target} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}${ARG_PRIVATE_HEADER_PATH}>
    )
  endif()

  if(TARGET ${constexpr_target})
    catch_discover_tests(
      ${constexpr_target}
      TEST_PREFIX
      "[${ARG_TARGET_PREFIX}:constexpr]-"
      REPORTER
      XML
      OUTPUT_DIR
      .
      OUTPUT_PREFIX
      "constexpr."
      OUTPUT_SUFFIX
      .xml
    )
  endif()

  if(TARGET ${relaxed_constexpr_target})
    catch_discover_tests(
      ${relaxed_constexpr_target}
      TEST_PREFIX
      "[${ARG_TARGET_PREFIX}:relaxed_constexpr]-"
      REPORTER
      XML
      OUTPUT_DIR
      .
      OUTPUT_PREFIX
      "relaxed_constexpr."
      OUTPUT_SUFFIX
      .xml
    )
  endif()
endfunction()
