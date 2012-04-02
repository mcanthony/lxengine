lx_glob_sources("include/lx0")
lx_glob_sources("src")

include_directories( src )
add_library( lxengine ${SOURCES} )

lx_symbolic_link( lxengine "common" "../../media" )

