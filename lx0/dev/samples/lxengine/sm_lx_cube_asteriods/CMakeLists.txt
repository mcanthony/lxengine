set(APP sm_lx_cube_asteriods)
simple_executable(${APP})

configure_file("level00.xml" "${CMAKE_BINARY_DIR}/data/${APP}/level00.xml" COPYONLY)
