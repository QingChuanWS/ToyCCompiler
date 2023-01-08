cmake_minimum_required(VERSION 3.17)

project(ToyC)
set (CMAKE_CXX_COMPILER "/usr/bin/g++-10")

add_compile_options(-std=c++11)

include_directories(.)

add_executable(toyc tools.cc object.cc node.cc type.cc main.cc codegen.cc token.cc)