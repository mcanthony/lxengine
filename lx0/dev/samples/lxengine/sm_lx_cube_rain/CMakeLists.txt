set(APPNAME sm_lx_cube_rain)

simple_executable( ${APPNAME} )

macro(lx_copy FILE)
    configure_file(${FILE} ${CMAKE_BINARY_DIR}/data/${APPNAME}/${FILE} COPYONLY)
endmacro(lx_copy)

configure_file("plugins_d.cfg" "${CMAKE_BINARY_DIR}/plugins_d.cfg" COPYONLY)
configure_file("plugins.cfg" "${CMAKE_BINARY_DIR}/plugins.cfg" COPYONLY)
