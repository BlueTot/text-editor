cmake_minimum_required(VERSION 3.10)
project(text_editor C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # ✅ Make sure this is here

include_directories(include)

file(GLOB SOURCES "src/*.c")
add_executable(text_editor ${SOURCES})

