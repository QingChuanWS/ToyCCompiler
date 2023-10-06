cmake_minimum_required(VERSION 3.17)

project(ToyC)

option(USE_GCC "Using GCC To Compile" ON)
option(USE_CLANG "Using Clang To Compile" OFF)

find_program(GCC_COMPILER gcc)
find_program(CLANG_COMPILER clang)

if(NOT USE_GCC AND NOT USE_CLANG)
  if(GCC_COMPILER)
    set(USE_GCC ON)
  elseif(CLANG_COMPILER)
    set(USE_CLANG ON)
  else()
    message(
      FATAL_ERROR "No available compiler found. Please install GCC or Clang.")
  endif()
endif()
if(USE_GCC)
  message("Using GCC To Compile")
  set(CMAKE_C_COMPILER "gcc")
  set(CMAKE_CXX_COMPILER "g++")
elseif(USE_CLANG)
  message("Using Clang To Compile")
  set(CMAKE_C_COMPILER "clang")
  set(CMAKE_CXX_COMPILER "clang++")
endif()

if(USE_GCC OR USE_CLANG)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
  if(NOT COMPILER_SUPPORTS_CXX17)
    message(
      FATAL_ERROR
        "The selected compiler does not support C++17 standard. Please upgrade your compiler or select a different one."
    )
  else()
    message("The selected compiler supports C++17 standard.")
  endif()
endif()

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
          ${CMAKE_SOURCE_DIR}/test/src ${CMAKE_CURRENT_BINARY_DIR}/test_output CMAKE_C_COMPILER)
set_tests_properties(function_test PROPERTIES FAIL_REGULAR_EXPRESSION
                                              "-:|Segmentation fault")

add_test(NAME driver_test
         COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/script/drivers.sh
                 ${CMAKE_CURRENT_BINARY_DIR})
