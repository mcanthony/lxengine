set(PLUGIN "OgreView")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/${PLUGIN}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/${PLUGIN}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/${PLUGIN}")

lx_glob_sources("src")
include_directories( src )

add_library( ${PLUGIN} SHARED ${SOURCES} )
target_link_libraries( ${PLUGIN}
    ${LX_ALL_LIBS}
    ${OGRE_LIBRARIES} 
    ) 
