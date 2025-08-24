arquebus_catch2_test_setup(
  TARGET_PREFIX lib_arquebus
  TEST_SOURCES shared_memory_tests.cpp
  PRIVATE_HEADER_PATH /lib/arquebus/include
  LINK_LIBRARIES arquebus::arquebus_options arquebus::arquebus_warnings arquebus::arquebus
)
