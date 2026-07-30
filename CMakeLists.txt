cmake_minimum_required(VERSION 3.23)
project(reakory2 VERSION 0.0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(deps/ext.cmake)

add_library(arena OBJECT
  src/Components/Allocators/Arena.cpp
  src/Components/Allocators/Arena.hpp
)
target_include_directories(arena PUBLIC src/Components/Allocators)

add_executable(main src/main.cpp)
target_include_directories(main PRIVATE src src/Components)
target_link_libraries(main PRIVATE arena ZLIB::ZLIB)
