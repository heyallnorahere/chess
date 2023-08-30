cmake_minimum_required(VERSION 3.20)
project(chess)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

include(CTest)
enable_testing()

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("tests")

# C# binding library
add_subdirectory("csharp/LibChess.Native")