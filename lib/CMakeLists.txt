cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE LIBCHESS_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
add_library(libchess STATIC ${LIBCHESS_SOURCE})

target_include_directories(libchess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(libchess PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libchesspch.h")
set_target_properties(libchess PROPERTIES
    CXX_STANDARD 17
    OUTPUT_NAME "chess"
    FOLDER "core")