
file(GLOB_RECURSE SRC_CPP src/*.cpp)
file(GLOB_RECURSE SRC_HPP include/*.hpp)
file(GLOB_RECURSE SRC_H include/*.h)
set(SOURCES ${SRC_CPP} ${SRC_H} ${SRC_HPP})

add_library( lxcanvas ${SOURCES} )
