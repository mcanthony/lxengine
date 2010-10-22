project(lx0)

#
# CMake configuration
#

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/config/cmake")

#
# Look for prebuilt binaries
#
IF(${MSVC10})
    SET(DEPS_SDK ${CMAKE_SOURCE_DIR}/dependencies/sdk/msvc10)
ELSEIF(${MSVC90})
    SET(DEPS_SDK ${CMAKE_SOURCE_DIR}/dependencies/sdk/msvc9)
ENDIF()

IF(EXISTS "${DEPS_SDK}")
    message(STATUS "Using pre-built dependencies in ${DEPS_SDK}")
    include(${CMAKE_SOURCE_DIR}/dependencies/IncludeDependencies.cmake)
ELSE()
    message (STATUS "Pre-built dependencies not found.  Relying on CMake to find the packages.")
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

# LxEngine

include_directories( libs/core/include )

# Boost
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})


# OGRE
message(STATUS "Ogre include dir: ${OGRE_INCLUDE_DIR}")
include_directories(${OGRE_INCLUDE_DIR})
link_directories (${OGRE_LIB_DIR}) 
file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/debug/*.dll")

# Note: is there any way to use INSTALL to install a renamed file?  Using a
# mix of install and configure_file seems confusing.
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR})
configure_file($ENV{OGRE_HOME}/bin/debug/plugins_d.cfg ${PROJECT_BINARY_DIR}/plugins.cfg COPYONLY)

#
# Helper macros
#

set( LX_ALL_LIBS ${OGRE_LIBRARIES} lx0 ${V8_LIBS} )

macro(simple_executable EXENAME)

   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
   
   file(GLOB_RECURSE SRCFILES *.cpp)
   file(GLOB_RECURSE HDRFILES *.h)
   
   set(SOURCES ${SRCFILES} ${HDRFILES})
   add_executable(${EXENAME} ${SOURCES})
   target_link_libraries(${EXENAME} ${LX_ALL_LIBS}) 

endmacro(simple_executable EXENAME)

# Recursively add all subdirectories containing a CMakeLists.txt.

macro(recurse_subdirectories BASEDIR)
   message(STATUS "Recursively adding subdirectories for: ${BASEDIR}")
   file(GLOB_RECURSE SUBFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${BASEDIR}/*/CMakeLists.txt")
   foreach(FILE ${SUBFILES})
      get_filename_component(FILE ${FILE} PATH)
      if (NOT ${FILE} EQUAL "")
         message(STATUS "    Adding ${FILE}")
         add_subdirectory( "${FILE}" )
      endif()
   endforeach(FILE)
endmacro(recurse_subdirectories BASEDIR)

#
# Sub-projects
#

recurse_subdirectories(libs)
recurse_subdirectories(samples)
recurse_subdirectories(sandbox)
recurse_subdirectories(unittest)

