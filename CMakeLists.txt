cmake_minimum_required(VERSION 3.20)
project(cpp_suffix_tree)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra)

add_executable(suffix_tree_example src/Example.cpp)