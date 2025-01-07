# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(tspp-compiler VERSION 1.0.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add source directory
add_subdirectory(src)

# Create a target to copy example files to build directory
# file(GLOB EXAMPLE_FILES "examples/*.xyz")
# foreach(EXAMPLE_FILE ${EXAMPLE_FILES})
#     file(COPY ${EXAMPLE_FILE} DESTINATION ${CMAKE_BINARY_DIR}/bin/examples)
# endforeach()