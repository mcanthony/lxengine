set(NAME "lxcore")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

lx_glob_sources("src")
lx_glob_sources("include/lx0/core2")
include_directories( src )
include_directories( include )

set(SOURCES ${SOURCES} include/lx0/core2.hpp)
source_group("Source" FILES include/lx0/core2.hpp)

add_library( ${NAME} ${SOURCES} )
