set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.hpp)
set(SOURCES ${SRCFILES} ${HDRFILES})
source_group("Source" FILES ${SOURCES})

add_executable(glgeom_unittest ${SOURCES})

