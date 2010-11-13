#
# Glob a directory and move all files in that directory into the same
# Visual Studio source grouping.  Then, if there is a corresponding
# header in the include/lx0 directory include that in the source grouping
# as well (to keep the cpp and header next to each other in the IDE).
#
macro(lx_source_group FOLDER)
    file(GLOB_RECURSE FILELIST1 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src src/${FOLDER}/*.cpp)
    file(GLOB_RECURSE FILELIST2 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src src/${FOLDER}/*.hpp)
    set(FILELIST ${FILELIST1} ${FILELIST2})
    foreach(F ${FILELIST})
        get_filename_component(DIR ${F} PATH)
        string(REPLACE "/" "\\" DIR ${DIR} )
        message(STATUS "DIR: ${DIR} ${F}")
        source_group(${DIR} FILES src/${F})
        
        get_filename_component(H ${F} NAME_WE)
        set(H "${CMAKE_CURRENT_SOURCE_DIR}/include/lx0/${H}.hpp")
        source_group(${DIR} FILES ${H})
    endforeach(F)
endmacro(lx_source_group)

file(GLOB_RECURSE TINYXML_SRC src/extern/tinyxml/*.cpp)
file(GLOB_RECURSE TINYXML_HDR src/extern/tinyxml/*.h)
set(EXTERN_SRC ${TINYXML_SRC} ${TINYXML_HDR})
source_group("extern/tinymxl" FILES ${TINYXML_SRC})
source_group("extern/tinymxl" FILES ${TINYXML_HDR})

file(GLOB_RECURSE SRCFILES src/*.cpp)
file(GLOB_RECURSE SRCHPPFILES src/*.hpp)
file(GLOB_RECURSE HFILES include/*.hpp)
file(GLOB_RECURSE HPPFILES include/*.h)
set(SOURCES ${SRCFILES} ${SRCHPPFILES} ${HFILES} ${HPPFILES} ${EXTERN_SRC})

lx_source_group("core")
lx_source_group("primitives")
lx_source_group("dom")
lx_source_group("engine")
lx_source_group("serial")
lx_source_group("script")
lx_source_group("util")
lx_source_group("views")

source_group("_detail" "include/lx0/detail/forward_decls.hpp")
source_group("_detail" "include/lx0/detail/dom_base.hpp")
source_group("_detail" "include/lx0/detail/dom_base.cpp")
source_group("core" include/lx0/cast.hpp)

add_library( lx0 ${SOURCES} )
