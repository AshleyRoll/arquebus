arquebus_catch2_test_setup(
  TARGET_PREFIX lib_arquebus
  TEST_SOURCES shared_memory_tests.cpp buffer_size_tests.cpp spsc/var_msg/producer_tests.cpp
  LINK_LIBRARIES arquebus::arquebus_options arquebus::arquebus_warnings arquebus::arquebus
)
