find_package(Boost COMPONENTS unit_test_framework REQUIRED)

mark_as_advanced(CLEAR Boost_UNIT_TEST_FRAMEWORK_LIBRARY_DEBUG)
mark_as_advanced(CLEAR Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE)

file(GLOB TEST_SRCS "test*.cpp")
foreach(TEST_SRC ${TEST_SRCS}) # e.g. "test_example1.cpp"
    get_filename_component(SHORT_NAME ${TEST_SRC} NAME_WE) # e.g. "test_example1"
    add_executable(${SHORT_NAME}
        ${TEST_SRC}
        ${MODAL_DOWNSAMPLE_SRCS})
    target_link_libraries(${SHORT_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    add_test(NAME ${SHORT_NAME} COMMAND ${SHORT_NAME})
endforeach()
