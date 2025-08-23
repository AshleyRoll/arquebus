# in case Git is not available, we default to "unknown"
set(GIT_HASH "unknown")

# find Git and if available set GIT_HASH variable
find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND git log -1 --pretty=format:%H
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )
endif()

string(SUBSTRING "${GIT_HASH}" 0 8 GIT_HASH_SHORT)

message(STATUS "Git hash is ${GIT_HASH}, short ${GIT_HASH_SHORT}")
