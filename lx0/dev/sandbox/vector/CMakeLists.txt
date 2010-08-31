set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.h)

set(SOURCES ${SRCFILES} ${HDRFILES})
add_executable(vector ${SOURCES})
target_link_libraries (vector ${OGRE_LIBRARIES}) 

