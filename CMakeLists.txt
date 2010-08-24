project(LxEngine)
cmake_minimum_required(VERSION 2.8)

#
# Project-wide settings
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${LxEngine_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LxEngine_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LxEngine_BINARY_DIR}")

# 
# Sub-projects
#
add_subdirectory( sandbox );