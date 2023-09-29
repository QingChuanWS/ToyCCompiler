cmake_minimum_required(VERSION 3.17)

project(ToyC)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")
add_compile_options(-std=c++11)

include_directories(src)
aux_source_directory(src SOURCES)
add_executable(toyc ${SOURCES})

enable_testing()

find_program(BASH_EXECUTABLE bash)

add_test(
  NAME memory_test
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/script/memory.sh
          ${CMAKE_SOURCE_DIR}/test/src ${CMAKE_CURRENT_BINARY_DIR}/test_output)
set_tests_properties(
  memory_test PROPERTIES FAIL_REGULAR_EXPRESSION
                         "memory error|Segmentation fault |Aborted")

add_test(
  NAME function_test
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/script/function.sh
          ${CMAKE_SOURCE_DIR}/test/src ${CMAKE_CURRENT_BINARY_DIR}/test_output)
set_tests_properties(function_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                              "-:|Segmentation fault")

add_test(NAME driver_test
         COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/script/drivers.sh
                 ${CMAKE_CURRENT_BINARY_DIR})
