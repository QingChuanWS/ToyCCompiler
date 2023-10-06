cmake_minimum_required(VERSION 3.17)

project(ToyC)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
if(NOT COMPILER_SUPPORTS_CXX17)
  message(
    FATAL_ERROR
      "The selected compiler does not support C++17 standard. Please upgrade your compiler or select a different one."
  )
else()
  message("-- The selected compiler supports C++17 standard.")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)
aux_source_directory(src SOURCES)
add_executable(toyc ${SOURCES})

# ------------------------test------------------------
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
          ${CMAKE_SOURCE_DIR}/test/src ${CMAKE_CURRENT_BINARY_DIR}/test_output ${CMAKE_C_COMPILER})
set_tests_properties(function_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                              "-:|Segmentation fault")

add_test(NAME driver_test
         COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/script/drivers.sh
                 ${CMAKE_CURRENT_BINARY_DIR})
