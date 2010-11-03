set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE SRCFILES *.cpp)
file(GLOB_RECURSE HDRFILES *.h)

set(SOURCES ${SRCFILES} ${HDRFILES})
add_executable(elm_function ${SOURCES})

#
# Copy data files
#
file(GLOB ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.elm)
foreach(FILE ${ASSETS})
    message(STATUS "Copying file ${FILE}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${FILE} ${CMAKE_BINARY_DIR}/data/${FILE} COPYONLY)
endforeach(FILE)
