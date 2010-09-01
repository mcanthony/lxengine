
file(GLOB_RECURSE SRCFILES src/*.cpp)
file(GLOB_RECURSE HFILES include/*.hpp)
file(GLOB_RECURSE HPPFILES include/*.h)

set(SOURCES ${SRCFILES} ${HFILES} ${HPPFILES})
source_group(src FILES ${SOURCES} )

add_library( lx0 ${SOURCES} )
