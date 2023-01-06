cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE CHESS_SOURCE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE RENDERER_BACKENDS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/renderer/*.cpp")
list(REMOVE_ITEM CHESS_SOURCE ${RENDERER_BACKENDS})

set(CHESS_LIBRARIES libchess)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(TARGET_PLATFORM "windows")
else()
    set(TARGET_PLATFORM "unix")
    list(APPEND CHESS_LIBRARIES pthread)
endif()

list(APPEND CHESS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/renderer/${TARGET_PLATFORM}_backend.cpp")
add_executable(chess ${CHESS_SOURCE})

target_link_libraries(chess PRIVATE ${CHESS_LIBRARIES})
target_include_directories(chess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(chess PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
set_target_properties(chess PROPERTIES
    CXX_STANDARD 17
    FOLDER "core")