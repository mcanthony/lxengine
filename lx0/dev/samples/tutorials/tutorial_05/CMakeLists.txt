set(PLUGIN "tutorial_05")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/apps/${PLUGIN}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/apps/${PLUGIN}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/apps/${PLUGIN}")
# file(COPY "data" DESTINATION "${PROJECT_BINARY_DIR}/apps/${PLUGIN}")

lx_glob_sources( src )
include_directories( src )

add_library( ${PLUGIN} SHARED ${SOURCES} )
add_dependencies( ${PLUGIN} lxengineapp )
target_link_libraries( ${PLUGIN} ${LX_ALL_LIBS} )
    
lx_symbolic_link(${PLUGIN} "apps/${PLUGIN}/data" "data")
