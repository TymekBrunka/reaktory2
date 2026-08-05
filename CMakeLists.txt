cmake_minimum_required(VERSION 3.23)
project(reakory2 VERSION 0.0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(deps/ext.cmake)
add_subdirectory(src/BuildTools)

imgpack_(
  OUTPUT images
  CHANNELS 4
  FLIP y
  SOURCES
    assets/images/icons.png
    assets/images/placeholder.png
)

imgpack_(
  OUTPUT icon_img
  CHANNELS 4
  FLIP n
  SOURCES
    assets/images/icon.png
)

imgpack_(
  OUTPUT placeholder_icon_img
  CHANNELS 4
  FLIP n
  SOURCES
    assets/images/placeholder.png
)

add_library(arena OBJECT
  src/Components/Allocators/Arena.cpp
  src/Components/Allocators/Arena.hpp
)
target_include_directories(arena PUBLIC src/Components/Allocators)

add_library(renderer STATIC
  src/Renderer/Renderer.cpp
  src/Renderer/Renderer_impl.cpp
  src/Renderer/Renderer.hpp
  src/Renderer/Renderer_internal.hpp
)
target_include_directories(renderer PUBLIC
  src/Renderer
  src/Components
)
target_link_libraries(renderer PRIVATE placeholder_icon_img glad glfw imgui)

add_executable(main src/main.cpp)
target_include_directories(main PRIVATE src src/Components)
target_link_libraries(main PRIVATE icon_img renderer imgui ZLIB::ZLIB)
