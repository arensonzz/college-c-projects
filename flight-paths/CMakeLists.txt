cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB TARGET_C "${CMAKE_SOURCE_DIR}/*.c")
project(flight_paths VERSION 1.0)
add_executable(flight_paths ${TARGET_C})
target_link_libraries(flight_paths m)
