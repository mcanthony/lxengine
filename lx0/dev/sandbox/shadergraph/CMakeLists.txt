set(APP sandbox_shadergraph)
simple_executable(${APP})

file(GLOB ASSETS *.lxfrag)
foreach(FILE ${ASSETS})
    file(RELATIVE_PATH RELFILE ${CMAKE_CURRENT_SOURCE_DIR} ${FILE})
    configure_file( ${FILE}  ${CMAKE_BINARY_DIR}/data/${APP}/${RELFILE}  COPYONLY)
endforeach(FILE)
