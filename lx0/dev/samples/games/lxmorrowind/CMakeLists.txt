set(APPNAME lxmorrowind)

simple_executable( ${APPNAME} )
target_link_libraries(${APPNAME} ${NIFLIB_LIBS} ${BULLET_LIBS}) 
