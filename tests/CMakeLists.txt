add_executable(tests
  test_string.cpp
  test_list.cpp
  test_set.cpp
  test_generic.cpp
  test_geo.cpp
)
target_link_libraries(tests PRIVATE labwork10_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests)
