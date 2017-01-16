project(ModalDownsampling LANGUAGES CXX)
cmake_minimum_required(VERSION 3.1)

# Definitely use our local include files
include_directories(include)

# We use boost::multi_array for n-dimensional label images
find_package(Boost REQUIRED)
mark_as_advanced(CLEAR Boost_INCLUDE_DIR) # Because finding Boost is important
include_directories(${Boost_INCLUDE_DIR})

# We use Qt for threading
find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

# Enumerate all of our local source files for easier linking
set(MODAL_DOWNSAMPLE_SRCS
	${CMAKE_SOURCE_DIR}/include/modal_downsample.hpp
	${CMAKE_SOURCE_DIR}/src/performance_parameters.hpp
	${CMAKE_SOURCE_DIR}/src/modal_downsample.cpp
)

add_subdirectory(src)

# Unit tests are optional
option(ENABLE_TESTING "Compile Unit Tests?" TRUE)
if(ENABLE_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
