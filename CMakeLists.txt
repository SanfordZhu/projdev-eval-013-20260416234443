cmake_minimum_required(VERSION 3.16)
project(map_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(code src/main.cpp)

# Include directories
target_include_directories(code PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
