#
# Relies on the DEPS_SDK variable being set to the directory that this file
# is in.
#

# Ogre
set(ENV{OGRE_HOME} "${DEPS_SDK}/ogre")

# Boost
set(ENV{BOOST_ROOT} "${DEPS_SDK}/boost")
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(ENV{BOOST_INCLUDEDIR} "$ENV{BOOST_ROOT}/include")
set(ENV{BOOST_LIBRARYDIR} "$ENV{BOOST_ROOT}/lib")

# V8
include_directories("${DEPS_SDK}/v8/include")
link_directories("${DEPS_SDK}/v8/lib")
set(V8_LIBS v8.lib ws2_32.lib winmm.lib)
