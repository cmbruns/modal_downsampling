find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_executable(test_example1 test_example1.cpp)
target_link_libraries(test_example1 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(NAME test_example1 COMMAND test_example1)
