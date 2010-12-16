
lx_glob_sources("src")
lx_glob_sources("include/lxcanvas")

add_definitions(-DGLEW_STATIC)
include_directories( src/extern/glew )
add_library( lxcanvas ${SOURCES} )
