project(lx0)

#
# CMake configuration
#
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/config/cmake")

#
# Look for prebuilt binaries
#
IF(EXISTS "${CMAKE_SOURCE_DIR}/external/prebuilt/msvc10-mtgd/OgreSDK_vc10_v1-7-1")
   set(PREBUILT_DIR "${CMAKE_SOURCE_DIR}/external/prebuilt/msvc10-mtgd")
   message(STATUS "Using pre-built binaries at: ${PREBUILT_DIR}")
   
   set(ENV{OGRE_HOME} "${PREBUILT_DIR}/OgreSDK_vc10_v1-7-1")
ENDIF()

#
# Dependencies
#
find_package(OGRE REQUIRED)

#
# Project-wide settings
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${lx0_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${lx0_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${lx0_BINARY_DIR}")

# 
# Sub-projects
#
add_subdirectory( samples )
add_subdirectory( sandbox )