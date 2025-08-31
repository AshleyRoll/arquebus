add_subdirectory(host)
add_subdirectory(producer)
add_subdirectory(consumer)

add_custom_target(
  examples ALL
  DEPENDS host producer consumer
  COMMENT "Used to group all example code into single target"
)
