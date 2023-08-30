cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE LIBCHESS_NATIVE_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
add_library(LibChess.Native SHARED ${LIBCHESS_NATIVE_SOURCE})
target_link_libraries(LibChess.Native PRIVATE libchess)
set_target_properties(LibChess.Native PROPERTIES
    CXX_STANDARD 17
    FOLDER "core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")