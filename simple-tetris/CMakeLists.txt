cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB TARGET_C "${CMAKE_SOURCE_DIR}/*.c")
project(simple_tetris VERSION 1.0)
add_executable(simple_tetris ${TARGET_C})

target_include_directories(simple_tetris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
