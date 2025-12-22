cmake_minimum_required(VERSION 3.10)

# Project Name
project(cheezy-engine)

# Use C++17 Standard (Required for things like if constexpr)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the src directory so headers (like position.h) can be found when included
include_directories(src)

# Automatically find all .cpp files in the src/ folder
file(GLOB SOURCES "src/*.cpp")

# Create the executable named 'cheezy-engine' from the found sources
add_executable(cheezy-engine ${SOURCES})
