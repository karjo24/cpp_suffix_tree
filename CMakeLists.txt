cmake_minimum_required(VERSION 3.20)
project(cpp_suffix_tree)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -------Example usage---------
add_compile_options(-Wall -Wextra)
add_executable(suffix_tree_example src/Example.cpp)
target_link_libraries(suffix_tree_example GTest::gtest_main)

# -------Tests---------
add_executable(tests tests/TestsSuffixTree.cpp)
target_link_libraries(tests GTest::gtest_main)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined)
target_link_options(tests PRIVATE -fsanitize=address,undefined)