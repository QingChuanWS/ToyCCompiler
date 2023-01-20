cmake_minimum_required(VERSION 3.17)

project(ToyC)
set (CMAKE_CXX_COMPILER "/usr/bin/g++-10")
add_compile_options(-std=c++11)

include_directories(include)
aux_source_directory(src SOURCES)
add_executable(toyc  ${SOURCES})

enable_testing()

add_test(
  NAME func_test
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/function.sh ${CMAKE_CURRENT_BINARY_DIR}
  )

add_test(
  NAME driver_test
  COMMAND sudo bash ${CMAKE_SOURCE_DIR}/test/drivers.sh ${CMAKE_CURRENT_BINARY_DIR}
  )

  