macro(lx_copy FILE)
    configure_file(${FILE} ${CMAKE_BINARY_DIR}/data/sm_lx_basic/${FILE} COPYONLY)
endmacro(lx_copy)


simple_executable( sm_lx_basic )
lx_copy(scene_000.xml)
lx_copy(script000.js)
lx_copy(lxquery.js)
lx_copy(smooth_cube.json)
lx_copy(tree.json)