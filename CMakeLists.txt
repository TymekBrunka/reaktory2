cmake_minimum_required(VERSION 3.23)
project(reakory2 VERSION 0.0.1)
include(deps/ext.cmake)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE ZLIB::ZLIB)

