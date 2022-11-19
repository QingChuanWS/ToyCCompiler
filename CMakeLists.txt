cmake_minimum_required(VERSION 3.17)

project(ToyC)

include_directories(.)

add_executable(toyc tools.cc node.cc main.cc codegen.cc token.cc)