include(deps/CPM.cmake)
set(CPM_USE_LOCAL_PACKAGES OFF)

macro (install)
endmacro ()

# macro (find_package)
# endmacro ()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  message("Ccache enabled")
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR} hash_dir=false)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR} hash_dir=false)
endif()

CPMAddPackage(
  NAME zlib
  VERSION 1.3.1.2
  GITHUB_REPOSITORY madler/zlib
  GIT_TAG v1.3.1.2
  OPTIONS
    "ZLIB_BUILD_SHARED OFF"
    "ZLIB_BUILD_STATIC ON"
    "ZLIB_BUILD_TESTING OFF"
    "ZLIB_INSTALL OFF"
)

set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
add_library(ZLIB::ZLIB ALIAS zlibstatic)

get_target_property(zlibincludes zlibstatic INCLUDE_DIRECTORIES)
set(ZLIB_INCLUDE_DIR ${zlibincludes} CACHE PATH "" FORCE)
#
set(OGRE_STATIC ON CACHE BOOL "" FORCE)
set(OGRE_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OGRE_BUILD_COMPONENT_OVERLAY_IMGUI ON CACHE BOOL "" FORCE)
set(OGRE_BITES_STATIC_PLUGINS ON CACHE BOOL "" FORCE)
set(OGRE_CONFIG_DOUBLE ON CACHE BOOL "" FORCE) 
 
CPMAddPackage( 
  NAME Ogre3D 
  VERSION 14.5.2 
  GITHUB_REPOSITORY OGRECave/ogre
  # GIT_TAG v14.5.2
  GIT_TAG 4879225c39370278d362528d2b30f4170b91a656
) 
 
message("ogre deps path: ${OGREDEPS_PATH}") 
 
# function(original_add_library) 
#   _add_library(${ARGV})
# endfunction()
#
# function(add_library target)
#   # if(target IN_LIST UNWANTED_TARGETS)
#   #   message(STATUS "Excluding library target: ${target}")
#   #   return()  # Skip creating the target
#   # endif()
#   if (target STREQUAL "SDL3::SDL3")
#     message("cnsdcnsdkjncdsc>>>>>>> sdl3 : ${ARGV2}")
#   endif()
#   if(target STREQUAL "SDL3::SDL3" AND ARGV2 STREQUAL "SDL3")
#     return()  # Skip creating the target
#   endif()
#   original_add_library(${ARGV})  # Create the target if allowed
# endfunction()
#

# set(THREADS_PREFER_PTHREAD_FLAG TRUE CACHE BOOL "" FORCE)
# # find_package(Threads REQUIRED)
# # message(${CMAKE_THREAD_LIBS_INIT})

# file(GLOB imgui_SRC
#   deps/imgui/*.h
#   deps/imgui/*.cpp
# )
# add_library(
#   imgui STATIC ${imgui_SRC}
# )
#
# CPMAddPackage(
#   NAME imguizmo
#   GITHUB_REPOSITORY CedricGuillemet/ImGuizmo
#   GIT_TAG 1.9
# )
# target_link_libraries(imguizmo PUBLIC imgui)

add_library(yyjson deps/yyjson/yyjson.c)
target_include_directories(yyjson PUBLIC deps/yyjson/)

# include(deps/excel.cmake)
