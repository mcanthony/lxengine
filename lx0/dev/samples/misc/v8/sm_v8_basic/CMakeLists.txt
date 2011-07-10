simple_executable( sm_v8_basic )
target_link_libraries ( sm_v8_basic ${V8_LIBS} ${LX_ALL_LIBS} ) 

configure_file(script.js ${CMAKE_BINARY_DIR}/script.js COPYONLY)