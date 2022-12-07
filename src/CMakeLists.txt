cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE CHESS_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_executable(chess ${CHESS_SOURCE})

target_link_libraries(chess PRIVATE libchess)
target_precompile_headers(chess PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
set_target_properties(chess PROPERTIES
    CXX_STANDARD 17
    FOLDER "core")