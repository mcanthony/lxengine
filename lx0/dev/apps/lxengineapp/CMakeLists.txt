set(APPNAME lxengineapp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
   
lx_glob_sources("src")      

add_executable(${APPNAME} ${SOURCES})
target_link_libraries(${APPNAME} ${LX_ALL_LIBS}) 