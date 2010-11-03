set(APPNAME sm_lx_cube_rain)



simple_executable( ${APPNAME} )

macro(lx_copy FILE)
    configure_file(${FILE} ${CMAKE_BINARY_DIR}/data/${APPNAME}/${FILE} COPYONLY)
endmacro(lx_copy)

lx_copy(scene_000.xml)

lx_copy(cube_rain.js)
lx_copy(lxquery.js)
lx_copy(lxquery_wind.js)

lx_copy(smooth_cube.json)
lx_copy(small_sphere.json)
lx_copy(unit_cube.json)
lx_copy(unit_sphere.json)
lx_copy(tree.json)

configure_file("data/sound/bounce/bounce.wav" "${CMAKE_BINARY_DIR}/data/${APPNAME}/bounce.wav" COPYONLY)
configure_file("data/music/germanic_peace_3/germanic_peace_3.ogg" "${CMAKE_BINARY_DIR}/data/${APPNAME}/germanic_peace_3.ogg" COPYONLY)
configure_file("data/ogre/overlay/FadeInOut.overlay" "${CMAKE_BINARY_DIR}/data/${APPNAME}/FadeInOut.overlay" COPYONLY)
configure_file("data/ogre/material/Overlay.material" "${CMAKE_BINARY_DIR}/data/${APPNAME}/Overlay.material" COPYONLY)
