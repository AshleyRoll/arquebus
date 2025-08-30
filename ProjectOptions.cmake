include(cmake/SystemLink.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

# determine if the current build environment supports sanitizers
macro(ARQUEBUS_SUPPORTS_SANITIZERS)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

# configure the build options for this project
macro(ARQUEBUS_SETUP_OPTIONS)
  option(ARQUEBUS_ENABLE_COVERAGE "Enable coverage reporting" OFF)

  arquebus_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR ARQUEBUS_PACKAGING_MAINTAINER_MODE)
    option(ARQUEBUS_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(ARQUEBUS_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(ARQUEBUS_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(ARQUEBUS_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(ARQUEBUS_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(ARQUEBUS_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(ARQUEBUS_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(ARQUEBUS_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(ARQUEBUS_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(ARQUEBUS_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(ARQUEBUS_ENABLE_CACHE "Enable ccache" OFF)
  else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      # debug builds
      option(ARQUEBUS_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
      option(ARQUEBUS_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
      option(ARQUEBUS_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
      option(ARQUEBUS_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
      option(ARQUEBUS_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
      option(ARQUEBUS_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
      option(ARQUEBUS_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
      option(ARQUEBUS_ENABLE_CACHE "Enable ccache" ON)
    else()
      # release builds
      option(ARQUEBUS_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
      option(ARQUEBUS_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
      option(ARQUEBUS_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
      option(ARQUEBUS_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
      option(ARQUEBUS_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
      option(ARQUEBUS_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
      option(ARQUEBUS_ENABLE_CACHE "Enable ccache" OFF)
    endif()
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      ARQUEBUS_WARNINGS_AS_ERRORS
      ARQUEBUS_ENABLE_USER_LINKER
      ARQUEBUS_ENABLE_SANITIZER_ADDRESS
      ARQUEBUS_ENABLE_SANITIZER_LEAK
      ARQUEBUS_ENABLE_SANITIZER_UNDEFINED
      ARQUEBUS_ENABLE_SANITIZER_THREAD
      ARQUEBUS_ENABLE_SANITIZER_MEMORY
      ARQUEBUS_ENABLE_UNITY_BUILD
      ARQUEBUS_ENABLE_CLANG_TIDY
      ARQUEBUS_ENABLE_CPPCHECK
      ARQUEBUS_ENABLE_COVERAGE
      ARQUEBUS_ENABLE_CACHE
    )
  endif()
endmacro()

# configure options local to this project
macro(ARQUEBUS_LOCAL_OPTIONS)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(arquebus_warnings INTERFACE)
  add_library(arquebus_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  arquebus_set_project_warnings(arquebus_warnings ${ARQUEBUS_WARNINGS_AS_ERRORS} "" "" "" "")

  if(ARQUEBUS_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(arquebus_options)
  endif()

  include(cmake/Sanitizers.cmake)
  arquebus_enable_sanitizers(
    arquebus_options ${ARQUEBUS_ENABLE_SANITIZER_ADDRESS} ${ARQUEBUS_ENABLE_SANITIZER_LEAK}
    ${ARQUEBUS_ENABLE_SANITIZER_UNDEFINED} ${ARQUEBUS_ENABLE_SANITIZER_THREAD} ${ARQUEBUS_ENABLE_SANITIZER_MEMORY}
  )

  set_target_properties(arquebus_options PROPERTIES UNITY_BUILD ${ARQUEBUS_ENABLE_UNITY_BUILD})

  if(ARQUEBUS_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    arquebus_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(ARQUEBUS_ENABLE_CLANG_TIDY)
    arquebus_enable_clang_tidy(arquebus_options ${ARQUEBUS_WARNINGS_AS_ERRORS})
  endif()

  if(ARQUEBUS_ENABLE_CPPCHECK)
    arquebus_enable_cppcheck(${ARQUEBUS_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(ARQUEBUS_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    arquebus_enable_coverage(arquebus_options)
  endif()

  if(ARQUEBUS_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now target_link_options(arquebus_options INTERFACE
      # -Wl,--fatal-warnings)
    endif()
  endif()
endmacro()
