set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.hpp)
file(GLOB_RECURSE INLFILES *.inl)
set(SOURCES ${SRCFILES} ${HDRFILES} ${INLFILES})
source_group("Source" FILES ${SOURCES})

# For convenience, add the glgeom headers to this project
file(GLOB_RECURSE GLGEOM_HDRFILES ../include/glgeom/*.hpp)
source_group("glgeom" FILES ${GLGEOM_HDRFILES})

add_executable(glgeom_unittest ${SOURCES} ${GLGEOM_HDRFILES})

