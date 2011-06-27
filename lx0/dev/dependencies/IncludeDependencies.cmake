#
# Relies on the DEPS_SDK variable being set to the directory that this file
# is in.
#

macro(install_debrel FOLDER FILE)
    MESSAGE(STATUS "Installing ${FILE}")
    INSTALL(FILES "${DEPS_SDK}/${FOLDER}/Debug/${FILE}" DESTINATION ${PROJECT_BINARY_DIR}/Debug)
    INSTALL(FILES "${DEPS_SDK}/${FOLDER}/Release/${FILE}" DESTINATION ${PROJECT_BINARY_DIR}/Release)
endmacro(install_debrel)

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
set(Boost_INCLUDE_DIR "$ENV{BOOST_ROOT}/include")
message("Set BOOST_ROOT = " $ENV{BOOST_ROOT})
message("Set Boost_INCLUDE_DIR = " ${Boost_INCLUDE_DIR})

include_directories("${DEPS_SDK}/boost/include")
link_directories("${DEPS_SDK}/boost/lib")

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
install_debrel("openal/bin" "alut.dll")

#
# Ogg Vorbis
#
include_directories("${DEPS_SDK}/libogg/include")
link_directories("${DEPS_SDK}/libogg/lib")
INSTALL(FILES "${DEPS_SDK}/libogg/bin/Debug/libogg.dll" DESTINATION ${PROJECT_BINARY_DIR})

include_directories("${DEPS_SDK}/libvorbis/include")
link_directories("${DEPS_SDK}/libvorbis/lib")
set(VORBIS_LIBS libvorbis.lib libvorbisfile.lib libogg.lib)

install_debrel("libvorbis/bin" "libvorbis.dll")
install_debrel("libvorbis/bin" "libvorbisfile.dll")

#
# GLM (OpenGL Mathematics)
#
include_directories("${DEPS_SDK}/glm/include")

