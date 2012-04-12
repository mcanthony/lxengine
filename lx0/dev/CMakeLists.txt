project(lx0)

#
# CMake configuration
#

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/config/cmake")

IF(MSVC)
   ADD_DEFINITIONS(/arch:SSE2 /fp:fast /Oi /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS)
   SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
   SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
ENDIF(MSVC)


#
# Look for prebuilt binaries.  LxEngine is designed to automatically
# build the dependent libraries into a uniform directory structure:
# this code is looking for those built binaries.
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

INSTALL(FILES "${CMAKE_SOURCE_DIR}/dependencies/openal_1_1/openal/OpenAL-Soft/build/Debug/OpenAL32.dll" DESTINATION ${PROJECT_BINARY_DIR}/Debug)
INSTALL(FILES "${CMAKE_SOURCE_DIR}/dependencies/openal_1_1/openal/OpenAL-Soft/build/Release/OpenAL32.dll" DESTINATION ${PROJECT_BINARY_DIR}/Release)

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
include_directories( libs/glgeom/include )
include_directories( libs/lxcore/include )
include_directories( libs/lxengine/include )
include_directories( libs/lxrasterizer/include )


# Boost
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

# OGRE
#
message(STATUS "Ogre include dir: ${OGRE_INCLUDE_DIR}")
include_directories(${OGRE_INCLUDE_DIR})
link_directories (${OGRE_LIB_DIR}) 

file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/debug/*.dll")
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR}/Debug)
file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/Release/*.dll")
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR}/Release)

include_directories( ${CMAKE_SOURCE_DIR}/libs/lxengine/src/extern/gl3w/include )

#
# Additional info
#
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")

#
# Helper macros
#

set( LX_ALL_LIBS 
    lxcore
    lxengine
    lxrasterizer
    ${V8_LIBS} 
    ${OIS_LIBS}     
    opengl32.lib
    glu32.lib
    )

macro(copy_media_file2 FILE FOLDER)
    configure_file( ${FILE}  ${CMAKE_BINARY_DIR}/${FOLDER}  COPYONLY)
endmacro(copy_media_file2)

macro(copy_media_file FILE)
    message(STATUS "Copying media: ${FILE}")
    configure_file( ${FILE}  ${CMAKE_BINARY_DIR}/${FILE}  COPYONLY)
endmacro(copy_media_file)

macro(copy_media_directory FOLDER)
    file(GLOB ASSETS ${FOLDER}/*)
    foreach(FILE ${ASSETS})
        if (NOT IS_DIRECTORY ${FILE})
            file(RELATIVE_PATH RELFILE ${CMAKE_SOURCE_DIR} ${FILE})
            copy_media_file(${RELFILE})
        endif()
    endforeach(FILE)
endmacro(copy_media_directory)

#
# MACRO: lx_glob_sources
#
# Recursively searches the BASE directory for *.c, *.cpp, *.h, *.hpp and
# appends matching files to the SOURCES variable.
#
# Automatically puts any found files in a Visual Studio source group
# under "Source/<path relative to BASE>".
#
macro(lx_glob_sources BASE)
    
    # Glob the files relative to the base, so the sub-path is easier to
    # extract
    file(GLOB_RECURSE SRC_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.c)
    file(GLOB_RECURSE SRC_H RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.h)
    file(GLOB_RECURSE SRC_CPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.cpp)
    file(GLOB_RECURSE SRC_HPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.hpp)
    set(SRC_ALL ${SRC_C} ${SRC_CPP} ${SRC_H} ${SRC_HPP})
     
    foreach (F ${SRC_ALL})
        
        # Put the file in the appropriate source group
        get_filename_component(DIR ${F} PATH)
        string(REPLACE "/" "\\" DIR "${DIR}" )
        source_group("Source\\${DIR}" FILES ${BASE}/${F})
    
        # Add the file to the SOURCES variable
        set(SOURCES ${SOURCES} "${BASE}/${F}")
    endforeach (F)

endmacro(lx_glob_sources BASE)

#
# MACRO: lx_symbolic_link
#
# Creates a symbolic link to a sub-directory of the current source directory
# in the project binary directory.
#
# Currently only works on Windows, Vista or later!
#
macro(lx_symbolic_link PROJECT LINK_TARGET LINK_SOURCE)
    
    FILE(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/${LINK_TARGET}" MKLINK_TARGET)
    FILE(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${LINK_SOURCE}" MKLINK_SOURCE)
    
    add_custom_command(TARGET ${PROJECT} 
        POST_BUILD
        COMMAND IF NOT EXIST "${MKLINK_TARGET}" mklink "/d" "${MKLINK_TARGET}" "${MKLINK_SOURCE}"
        COMMENT "Adding symbolic link"
    )
endmacro(lx_symbolic_link TARGET SOURCE)

#
# MACRO: simple_executable
# 
macro(simple_executable EXENAME)

   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
   
   lx_glob_sources(".")
      
   add_executable(${EXENAME} ${SOURCES})
   target_link_libraries(${EXENAME} ${LX_ALL_LIBS}) 

endmacro(simple_executable EXENAME)

macro(sample_executable EXENAME)
   simple_executable(${EXENAME})
   SET_PROPERTY(TARGET ${EXENAME} PROPERTY FOLDER "Sandbox")
endmacro(sample_executable EXENAME)


# Recursively add all subdirectories containing a CMakeLists.txt.

macro(recurse_subdirectories BASEDIR)
   message(STATUS "Recursively adding subdirectories for: ${BASEDIR}")
   file(GLOB_RECURSE SUBFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${BASEDIR}/*/CMakeLists.txt")
   foreach(FILE ${SUBFILES})
      get_filename_component(FILE ${FILE} PATH)
      if (NOT ${FILE} EQUAL "")
         message(STATUS "    Adding ${FILE}")
         include_directories( "${FILE}/include" )
         add_subdirectory( "${FILE}" )
      endif()
   endforeach(FILE)
endmacro(recurse_subdirectories BASEDIR)


macro(recurse_subdirectories_includes BASEDIR)
   message(STATUS "Recursively adding includes for: ${BASEDIR}")
   file(GLOB_RECURSE SUBFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${BASEDIR}/*/CMakeLists.txt")
   foreach(FILE ${SUBFILES})
      get_filename_component(FILE ${FILE} PATH)
      if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}/include/")
         message(STATUS "    Including ${FILE}/include")
         include_directories( "${FILE}/include" )
      endif()
   endforeach(FILE)
endmacro(recurse_subdirectories_includes BASEDIR)

#
# Sub-projects
#

recurse_subdirectories_includes(plugins)

add_subdirectory( libs/lxcore )
add_subdirectory( libs/lxengine )
add_subdirectory( libs/lxengine/unittest )
add_subdirectory( libs/glgeom/unittest )
add_subdirectory( libs/glgeom/benchmarks )
add_subdirectory( libs/lxrasterizer )

recurse_subdirectories(apps)
recurse_subdirectories(plugins)
recurse_subdirectories(benchmarks)
recurse_subdirectories(samples)
recurse_subdirectories(sandbox)
recurse_subdirectories(unittest)

#
# Organize projects into folders
#
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET lxengineapp             PROPERTY FOLDER "Apps/LxEngineApp")

SET_PROPERTY(TARGET lxcore                  PROPERTY FOLDER "Libs")
SET_PROPERTY(TARGET lxengine                PROPERTY FOLDER "Libs")
SET_PROPERTY(TARGET glgeom_benchmark        PROPERTY FOLDER "Libs")
SET_PROPERTY(TARGET glgeom_unittest         PROPERTY FOLDER "Libs")
SET_PROPERTY(TARGET lxrasterizer            PROPERTY FOLDER "Libs")

SET_PROPERTY(TARGET SoundAL                 PROPERTY FOLDER "Plugins")
SET_PROPERTY(TARGET OgreView                PROPERTY FOLDER "Plugins")
SET_PROPERTY(TARGET BulletPhysics           PROPERTY FOLDER "Plugins")

SET_PROPERTY(TARGET sm_lx_cube_asteriods    PROPERTY FOLDER "Samples/LxEngine")
SET_PROPERTY(TARGET sm_lx_cube_rain         PROPERTY FOLDER "Samples/LxEngine")
SET_PROPERTY(TARGET sm_lxcanvas             PROPERTY FOLDER "Samples/LxEngine")
SET_PROPERTY(TARGET sm_rasterizer           PROPERTY FOLDER "Samples/LxEngine")
SET_PROPERTY(TARGET sm_raytracer            PROPERTY FOLDER "Samples/LxEngine")
SET_PROPERTY(TARGET sm_terrain              PROPERTY FOLDER "Samples/LxEngine")

SET_PROPERTY(TARGET lxcraft                 PROPERTY FOLDER "Samples/Games")
SET_PROPERTY(TARGET lxquake2                PROPERTY FOLDER "Samples/Games")
SET_PROPERTY(TARGET lxmorrowind             PROPERTY FOLDER "Samples/Games")

SET_PROPERTY(TARGET tutorial_00             PROPERTY FOLDER "Samples/Tutorials")
SET_PROPERTY(TARGET tutorial_01             PROPERTY FOLDER "Samples/Tutorials")
SET_PROPERTY(TARGET tutorial_02             PROPERTY FOLDER "Samples/Tutorials")
SET_PROPERTY(TARGET tutorial_03             PROPERTY FOLDER "Samples/Tutorials")
SET_PROPERTY(TARGET tutorial_04             PROPERTY FOLDER "Samples/Tutorials")
SET_PROPERTY(TARGET tutorial_05             PROPERTY FOLDER "Samples/Tutorials")

SET_PROPERTY(TARGET sm_ogre_minimal         PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET sm_v8_basic             PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET cpp_smartptr            PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET cpp_remove_duplicates   PROPERTY FOLDER "Samples/Misc")

SET_PROPERTY(TARGET procedural2d            PROPERTY FOLDER "Samples/Tools")

SET_PROPERTY(TARGET bm_lxvar                    PROPERTY FOLDER "Benchmarks/lxcore")
SET_PROPERTY(TARGET primitive_buffer_adjacency  PROPERTY FOLDER "Benchmarks/GLGeom")

SET_PROPERTY(TARGET blendload               PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET elm_reference           PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET elm_function            PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sandbox_shadergraph     PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_fixedpoint           PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_unittest_app         PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_unittest_patterns    PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET unittest_lxengine       PROPERTY FOLDER "UnitTest")
