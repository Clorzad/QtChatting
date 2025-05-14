cmake_minimum_required(VERSION 3.20)

project(qt_chatting_project LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBRARY_OUTPUT_DIRECTORY )

add_subdirectory(gate_server)
add_subdirectory(proto)