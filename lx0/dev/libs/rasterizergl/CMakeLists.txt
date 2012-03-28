set(NAME "RasterizerGL")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

lx_glob_sources("src")
lx_glob_sources("include/lx0/libs")
include_directories( src )
include_directories( include )

add_library( ${NAME} ${SOURCES} )
target_link_libraries( ${NAME} 
    ${LX_ALL_LIBS}
    ) 
