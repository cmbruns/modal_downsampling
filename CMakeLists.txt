project(ModalDownsampling LANGUAGES CXX)
cmake_minimum_required(VERSION 2.4)

include_directories(include)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)

option(ENABLE_TESTING "Compile Unit Tests?" TRUE)
if(ENABLE_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
