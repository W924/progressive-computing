cmake_minimum_required(VERSION 3.0)

add_compile_options(-std=c++11 -fopenmp -DCPU_ONLY -march=armv8-a)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Euler.cmake)

project(test)


aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST) 


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# c++ standard >= 11
set(CMAKE_CXX_STANDARD 11)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)


add_executable(main ${SRC_LIST})


