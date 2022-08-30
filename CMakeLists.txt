cmake_minimum_required(VERSION 3.0.0)
project(libfptree VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(test_alpha tests/test_alpha.cpp)
add_executable(test_custom_struct tests/test_custom_struct.cpp)

add_test(NAME test_alpha COMMAND $<TARGET_FILE:test_alpha>)
add_test(NAME test_custom_struct COMMAND $<TARGET_FILE:test_custom_struct>)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
