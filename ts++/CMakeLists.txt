# CMakeLists.txt (root)
cmake_minimum_required(VERSION 3.15)
project(tspp VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test programs" ON)

# Add source directory
add_subdirectory(src)

# Add tests if enabled
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()