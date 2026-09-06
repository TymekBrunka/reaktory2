cmake_minimum_required(VERSION 3.23)
project(reakory2 VERSION 0.0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (SANS)
  set(CMAKE_CXX_FLAGS -fsanitize=address,undefined,memory)
  set(CMAKE_C_FLAGS -fsanitize=address,undefined,memory)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  if (CMAKE_C_COMPILER_ID MATCHES "MSVC") # not sure if this project will build on msvc but regardless, ill set this flags for it just in case

    if(NOT CMAKE_C_FLAGS_RELEASE)
      set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG")
    endif()
    if(NOT CMAKE_CXX_FLAGS_RELEASE)
      set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    endif()

  else()

    if(NOT CMAKE_C_FLAGS_RELEASE)
      set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
    endif()
    if(NOT CMAKE_CXX_FLAGS_RELEASE)
      set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    endif()

  endif()
endif()

include(deps/ext.cmake)
add_subdirectory(src/BuildTools)

bytepack_(
  OUTPUT shaders
  SOURCES
    assets/shaders/simple.vs
    assets/shaders/simple.fs
    assets/shaders/skybox.vs
    assets/shaders/skybox.fs
    # assets/shaders/skinless.vs
    assets/shaders/skinning.vs
    assets/shaders/skinning.fs
)

bytepack_(
  OUTPUT staticassets_scene
  SOURCES
    assets/example/document.json
    assets/example/scene.json
    assets/example/skybox.png
    assets/example/excel_sheets/data.xlsx
)

imgpack_(
  OUTPUT images
  CHANNELS 4
  FLIP y
  SOURCES
    assets/images/icons.png
    assets/images/placeholder.png
)

bytepack_(
  OUTPUT images_bigger
  SOURCES
    assets/images/scene_new.png
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
    assets/images/placeholder2.png
)

add_library(pch_core STATIC
  src/precompiled_headers/core.hpp
  src/precompiled_headers/core.cpp
)
target_include_directories(pch_core PUBLIC src/precompiled_headers)
target_link_libraries(pch_core PUBLIC concurrentqueue)
target_precompile_headers(pch_core PUBLIC src/precompiled_headers/core.hpp)

# add_library(allocators STATIC
#   src/Components/Allocators/Arena.cpp
#   src/Components/Allocators/Arena.hpp
# )
# target_include_directories(allocators PUBLIC src/Components/Allocators)
# target_link_libraries(allocators PUBLIC mimalloc-static)

add_library(logging STATIC
  src/Components/Logging/Logging.cpp
  src/Components/Logging/Logging.hpp
)

add_library(translations STATIC
  src/Components/Logging/Translations.cpp
  src/Components/Logging/Translations.hpp
)

target_include_directories(translations PUBLIC src/Components/Logging)
target_link_libraries(translations PUBLIC pch_core yyjson mimalloc-static)
# target_precompile_headers(translations REUSE_FROM pch_core)

target_include_directories(logging PUBLIC src/Components/Logging)
target_link_libraries(logging PUBLIC translations)
# target_precompile_headers(logging REUSE_FROM pch_core)
target_precompile_headers(logging PUBLIC src/Components/Logging/Logging.hpp)

target_precompile_headers(translations REUSE_FROM logging)

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
target_link_libraries(renderer
  PRIVATE
  placeholder_icon_img glad imgui
  PUBLIC
  glfw logging
)
# target_precompile_headers(renderer REUSE_FROM pch_core)
target_precompile_headers(renderer REUSE_FROM logging)

add_library(renderer_model STATIC
  src/Renderer/Model.cpp
  src/Renderer/Model.hpp
  src/Renderer/AssimpGLMHelpers.hpp
)
target_include_directories(renderer_model PUBLIC
  src/Renderer
  src/Components
)
target_link_libraries(renderer_model PUBLIC glad glm::glm PRIVATE logging assimp)

add_library(fileutils STATIC
  src/Components/FileUtils/FileUtils.cpp
  src/Components/FileUtils/FileUtils.hpp
)
target_include_directories(fileutils PUBLIC
  src/Components/FileUtils
  src/Components
)
target_precompile_headers(renderer_model REUSE_FROM logging)

add_library(Scene STATIC src/Scene/Scene.hpp src/Scene/Scene.cpp)
target_include_directories(Scene PUBLIC src/Scene/)
target_link_libraries(Scene PUBLIC logging renderer renderer_model fileutils glm::glm PRIVATE staticassets_scene shaders glad imgui)
target_precompile_headers(Scene REUSE_FROM logging)

add_library(App STATIC src/App.hpp src/App.cpp)
target_include_directories(App PUBLIC src)
target_link_libraries(App PUBLIC Scene icon_img logging renderer imgui fileutils images images_bigger)
target_precompile_headers(App REUSE_FROM logging)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE App)
# target_include_directories(main PRIVATE src src/Components)
# target_precompile_headers(main REUSE_FROM pch_core)
target_precompile_headers(main REUSE_FROM logging)
