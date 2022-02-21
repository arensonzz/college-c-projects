cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB TARGET_C "${CMAKE_SOURCE_DIR}/*.c")
project(college_database_system VERSION 1.0)
add_executable(college_database_system ${TARGET_C})

target_include_directories(college_database_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
