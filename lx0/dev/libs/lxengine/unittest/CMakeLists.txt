set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

macro(glgeom_group_sources BASE)
    
    # Glob the files relative to the base, so the sub-path is easier to
    # extract
    file(GLOB_RECURSE SRC_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.c)
    file(GLOB_RECURSE SRC_H RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.h)
    file(GLOB_RECURSE SRC_CPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.cpp)
    file(GLOB_RECURSE SRC_HPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.hpp)
    file(GLOB_RECURSE SRC_INL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${BASE} ${BASE}/*.inl)
    set(SRC_ALL ${SRC_C} ${SRC_CPP} ${SRC_H} ${SRC_HPP} ${SRC_INL})
     
    foreach (F ${SRC_ALL})
        
        # Put the file in the appropriate source group
        get_filename_component(DIR ${F} PATH)
        string(REPLACE "/" "\\" DIR "${DIR}" )
        source_group("Source\\${DIR}" FILES ${BASE}/${F})
    
        # Add the file to the SOURCES variable
        set(SOURCES ${SOURCES} "${BASE}/${F}")
    endforeach (F)

endmacro(glgeom_group_sources BASE)

glgeom_group_sources(src)
include_directories(src)

add_executable(unittest_lxengine ${SOURCES})
target_link_libraries(unittest_lxengine ${LX_ALL_LIBS} ) 
