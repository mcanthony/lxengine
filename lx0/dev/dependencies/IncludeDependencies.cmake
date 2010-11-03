#
# Relies on the DEPS_SDK variable being set to the directory that this file
# is in.
#


#
# Ogre
#
set(ENV{OGRE_HOME} "${DEPS_SDK}/ogre")

#
# OIS
#
include_directories("${DEPS_SDK}/ois/include")
link_directories("${DEPS_SDK}/ois/lib")
set(OIS_LIBS 
    debug       OIS_static_d.lib 
    optimized   OIS_static.lib)

#
# Boost
#
set(ENV{BOOST_ROOT} "${DEPS_SDK}/boost")
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(ENV{BOOST_INCLUDEDIR} "$ENV{BOOST_ROOT}/include")
set(ENV{BOOST_LIBRARYDIR} "$ENV{BOOST_ROOT}/lib")

#
# V8
#
include_directories("${DEPS_SDK}/v8/include")
link_directories("${DEPS_SDK}/v8/lib")
set(V8_LIBS 
        debug       v8_g.lib 
        optimized   v8.lib 
        general     ws2_32.lib 
        general     winmm.lib)

# Bullet
include_directories("${DEPS_SDK}/bullet/include")
link_directories("${DEPS_SDK}/bullet/lib")
set(BULLET_LIBS 
    BulletCollision.lib 
    BulletDynamics.lib 
    LinearMath.lib)

# OpenAL
include_directories("${DEPS_SDK}/openal/include")
include_directories("${DEPS_SDK}/openal/include/AL")    # Unfortunately necessary for alut.h's references to al.h
link_directories("${DEPS_SDK}/openal/lib")
set(OPENAL_LIBS 
    OpenAL32.lib 
    alut.lib)
INSTALL(FILES "${DEPS_SDK}/openal/bin/${CMAKE_BUILD_TYPE}/alut.dll" DESTINATION ${PROJECT_BINARY_DIR})


#
# Ogg Vorbis
#
include_directories("${DEPS_SDK}/libogg/include")
link_directories("${DEPS_SDK}/libogg/lib")
INSTALL(FILES "${DEPS_SDK}/libogg/bin/${CMAKE_BUILD_TYPE}/libogg.dll" DESTINATION ${PROJECT_BINARY_DIR})

include_directories("${DEPS_SDK}/libvorbis/include")
link_directories("${DEPS_SDK}/libvorbis/lib")
set(VORBIS_LIBS libvorbis.lib libvorbisfile.lib libogg.lib)

INSTALL(FILES "${DEPS_SDK}/libvorbis/bin/${CMAKE_BUILD_TYPE}/libvorbis.dll" DESTINATION ${PROJECT_BINARY_DIR})
INSTALL(FILES "${DEPS_SDK}/libvorbis/bin/${CMAKE_BUILD_TYPE}/libvorbisfile.dll" DESTINATION ${PROJECT_BINARY_DIR})

