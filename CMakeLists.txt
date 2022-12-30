cmake_minimum_required(VERSION 3.17)

project(ToyC)

include_directories(.)

add_executable(toyc tools.cc object.cc node.cc type.cc main.cc codegen.cc token.cc)