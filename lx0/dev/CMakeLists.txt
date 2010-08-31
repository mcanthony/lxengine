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
 
   set(ENV{BOOST_ROOT} "${PREBUILT_DIR}/boost_1_42_0")
   set(ENV{BOOST_LIBRARYDIR} "$ENV{BOOST_ROOT}/stage/lib")

ELSE()
    message (STATUS "Pre-built binaries not found.  Relying on CMake Find modules to locate dependencies.")
ENDIF()
#
# Dependencies
#
find_package(OGRE REQUIRED)
find_package(BOOST REQUIRED)

#
# Project-wide settings
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Boost
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

# OGRE
include_directories(${OGRE_INCLUDE_DIR})
link_directories (${OGRE_LIB_DIR}) 
file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/debug/*.dll")

# Note: is there any way to use INSTALL to install a renamed file?  Using a
# mix of install and configure_file seems confusing.
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR})
configure_file($ENV{OGRE_HOME}/bin/debug/plugins_d.cfg ${PROJECT_BINARY_DIR}/plugins.cfg COPYONLY)

# 
# Sub-projects
#
add_subdirectory( samples )
add_subdirectory( sandbox )