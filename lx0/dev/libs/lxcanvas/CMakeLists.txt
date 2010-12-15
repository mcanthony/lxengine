
file(GLOB_RECURSE SRC_C src/*.c)
file(GLOB_RECURSE SRC_H2 src/*.h)
file(GLOB_RECURSE SRC_H include/*.h)
file(GLOB_RECURSE SRC_CPP src/*.cpp)
file(GLOB_RECURSE SRC_HPP include/*.hpp)
set(SOURCES ${SRC_C} ${SRC_CPP} ${SRC_H} ${SRC_H2} ${SRC_HPP})

add_definitions(-DGLEW_STATIC)
include_directories( src/extern/glew )
add_library( lxcanvas ${SOURCES} )
