lx_glob_sources("include/lx0")
lx_glob_sources("src")

add_definitions(-DGLEW_STATIC)
include_directories( src )
add_library( lxengine ${SOURCES} )
