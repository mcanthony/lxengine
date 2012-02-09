set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/SoundAL")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/SoundAL")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Plugins/SoundAL")

lx_glob_sources("src")

include_directories( src )
add_library( soundal SHARED ${SOURCES} )
