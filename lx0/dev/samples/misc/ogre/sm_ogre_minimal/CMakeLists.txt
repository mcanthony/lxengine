set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.h)

set(SOURCES ${SRCFILES} ${HDRFILES})
add_executable(sm_ogre_minimal ${SOURCES})
target_link_libraries (sm_ogre_minimal ${OGRE_LIBRARIES}) 
