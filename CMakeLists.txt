cmake_minimum_required(VERSION 3.28)
project(itch_parser)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(itch_parser main.cpp MappedFile.h)

target_compile_options(itch_parser PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
)
