cmake_minimum_required(VERSION 3.20)

set(TESTBED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testbed")
file(GLOB TESTBED_SOURCES CONFIGURE_DEPENDS "${TESTBED_DIR}/*.cpp" "${TESTBED_DIR}/*.h")
add_library(testbed STATIC ${TESTBED_SOURCES})
set(TEST_TARGET_NAMES testbed)

target_link_libraries(testbed PUBLIC libchess)
target_include_directories(testbed PUBLIC ${TESTBED_DIR})

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach (TEST_SOURCE ${TEST_SOURCES})
    cmake_path(GET TEST_SOURCE FILENAME SOURCE_FILENAME)
    cmake_path(REMOVE_EXTENSION SOURCE_FILENAME OUTPUT_VARIABLE TEST_NAME)
    set(TEST_TARGET_NAME libchess_test_${TEST_NAME})

    add_executable(${TEST_TARGET_NAME} ${TEST_SOURCE})
    add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})

    target_link_libraries(${TEST_TARGET_NAME} PRIVATE testbed)
    list(APPEND TEST_TARGET_NAMES ${TEST_TARGET_NAME})
endforeach()

set_target_properties(${TEST_TARGET_NAMES} PROPERTIES
    CXX_STANDARD 17
    FOLDER "tests")