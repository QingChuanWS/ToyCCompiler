cmake_minimum_required(VERSION 3.17)

project(ToyC)
set (CMAKE_CXX_COMPILER "/usr/bin/g++-10")
add_compile_options(-std=c++11)

include_directories(.)
aux_source_directory(. SOURCES)

add_executable(toyc  ${SOURCES})

enable_testing()

add_test(
  NAME func_test
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test_function.sh
  )

add_test(
  NAME driver_test
  COMMAND sudo bash ${CMAKE_SOURCE_DIR}/test_drivers.sh
  )

  