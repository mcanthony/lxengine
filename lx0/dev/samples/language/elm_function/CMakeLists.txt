file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.h)

set(SOURCES ${SRCFILES} ${HDRFILES})
add_executable(elm_function ${SOURCES})
