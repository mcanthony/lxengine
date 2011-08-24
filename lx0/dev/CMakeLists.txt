project(lx0)

#
# CMake configuration
#

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/config/cmake")

IF(MSVC)
   ADD_DEFINITIONS(/arch:SSE2 /fp:fast /Oi /D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)


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
include_directories( libs/lxengine/include )
include_directories( libs/glgeom/include )

# Boost
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

# Modified Gl3w
include_directories( ${CMAKE_SOURCE_DIR}/libs/lxengine/src/extern/gl3w/include )

# OGRE
message(STATUS "Ogre include dir: ${OGRE_INCLUDE_DIR}")
include_directories(${OGRE_INCLUDE_DIR})
link_directories (${OGRE_LIB_DIR}) 
file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/debug/*.dll")
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR}/Debug)
file(GLOB ASSETS "$ENV{OGRE_HOME}/bin/Release/*.dll")
INSTALL(FILES ${ASSETS} DESTINATION ${PROJECT_BINARY_DIR}/Release)

message(STATUS "CMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")

#
# Helper macros
#

set( LX_ALL_LIBS 
    lxengine
    ${OGRE_LIBRARIES} 
    ${V8_LIBS} 
    ${BULLET_LIBS} 
    ${OIS_LIBS} 
    ${OPENAL_LIBS} 
    ${VORBIS_LIBS}
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
         add_subdirectory( "${FILE}" )
      endif()
   endforeach(FILE)
endmacro(recurse_subdirectories BASEDIR)

#
# Sub-projects
#

#recurse_subdirectories(libs)
add_subdirectory( libs/lxengine )
add_subdirectory( libs/lxengine/unittest )
add_subdirectory( libs/glgeom/unittest )
add_subdirectory( libs/glgeom/benchmarks )

recurse_subdirectories(benchmarks)
recurse_subdirectories(samples)
recurse_subdirectories(sandbox)
recurse_subdirectories(unittest)


#
# Organize projects into folders
#
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET lxengine                PROPERTY FOLDER "Libs/LxEngine")
SET_PROPERTY(TARGET glgeom_benchmark        PROPERTY FOLDER "Libs/GLGeom")
SET_PROPERTY(TARGET glgeom_unittest         PROPERTY FOLDER "Libs/GLGeom")

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

SET_PROPERTY(TARGET sm_ogre_minimal         PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET sm_v8_basic             PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET cpp_smartptr            PROPERTY FOLDER "Samples/Misc")
SET_PROPERTY(TARGET cpp_remove_duplicates   PROPERTY FOLDER "Samples/Misc")

SET_PROPERTY(TARGET procedural2d            PROPERTY FOLDER "Samples/Tools")

SET_PROPERTY(TARGET bm_lxvar                PROPERTY FOLDER "Benchmarks")
SET_PROPERTY(TARGET blendload               PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET elm_reference           PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET elm_function            PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sandbox_shadergraph     PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_fixedpoint           PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_unittest_app         PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET sb_unittest_patterns    PROPERTY FOLDER "Sandbox")
SET_PROPERTY(TARGET unittest_lxengine       PROPERTY FOLDER "UnitTest")
