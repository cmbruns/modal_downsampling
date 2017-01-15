find_package(Boost COMPONENTS unit_test_framework REQUIRED)

mark_as_advanced(CLEAR Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG)
mark_as_advanced(CLEAR Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE)

add_executable(test_example1 
	test_example1.cpp 
	${MODAL_DOWNSAMPLE_SRCS})
target_link_libraries(test_example1 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
# if you are using Linux, don’t forget to link against pthread and rt
if (UNIX AND NOT APPLE)
	target_link_libraries(test_example1 Threads::Threads rt_lib)
endif()
add_test(NAME test_example1 COMMAND test_example1)
