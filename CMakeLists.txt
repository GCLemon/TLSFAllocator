cmake_minimum_required(VERSION 3.8)

# make ZERO_CHECK disabled
set(CMAKE_SUPPRESS_REGENERATION true)

# solution name
project(TLSFAllocator)

# specify project type
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# specify destionation directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/DEBUG")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/DEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/RELEASE")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/RELEASE")

# set c++ flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# add directories
add_subdirectory(src)
add_subdirectory(test)