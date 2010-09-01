
file(GLOB_RECURSE SRCFILES src/*.cpp)
file(GLOB_RECURSE HDRFILES include/*.h)
   
set(SOURCES ${SRCFILES} ${HDRFILES})
add_library( lx0 ${SOURCES} )
