project(ModalDownsampling LANGUAGES CXX)
cmake_minimum_required(VERSION 3.1)

include_directories(include)

# if you are using Linux, don’t forget to link against pthread and rt
if (UNIX AND NOT APPLE)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	find_library(rt_lib rt)
endif()

find_package(Boost REQUIRED)
mark_as_advanced(CLEAR Boost_INCLUDE_DIR) # Because finding Boost is important
include_directories(${Boost_INCLUDE_DIR})

set(MODAL_DOWNSAMPLE_SRCS
	${CMAKE_SOURCE_DIR}/include/modal_downsample.hpp
	${CMAKE_SOURCE_DIR}/src/performance_parameters.hpp
	${CMAKE_SOURCE_DIR}/src/modal_downsample.cpp
)

add_subdirectory(src)

option(ENABLE_TESTING "Compile Unit Tests?" TRUE)
if(ENABLE_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
