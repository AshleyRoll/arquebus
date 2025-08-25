include(cmake/CPM.cmake)

# setup dependencies
#
# Done as a function so that updates to variables like CMAKE_CXX_FLAGS don't propagate out to other targets
function(arquebus_setup_dependencies)

  # For each dependency, see if it's already been provided to us by a parent project

  if(NOT TARGET fmt::fmt)
    cpmaddpackage("gh:fmtlib/fmt#11.2.0")
  endif()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage("gh:catchorg/Catch2@3.9.1")
  endif()

endfunction()
