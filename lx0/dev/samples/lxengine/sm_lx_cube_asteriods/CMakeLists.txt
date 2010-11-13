set(APP sm_lx_cube_asteriods)
simple_executable(${APP})

configure_file(${CMAKE_SOURCE_DIR}/media/models/space-ship-ship_00/ship_00-001.lxson ${CMAKE_BINARY_DIR}/media/models/ship_00.lxson COPYONLY)
configure_file("level00.xml" "${CMAKE_BINARY_DIR}/data/${APP}/level00.xml" COPYONLY)
