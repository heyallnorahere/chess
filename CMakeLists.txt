cmake_minimum_required(VERSION 3.20)
project(chess)

include(CTest)
enable_testing()

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("tests")