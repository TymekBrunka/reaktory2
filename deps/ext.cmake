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

message(zlib)
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

message(rpmalloc)
CPMAddPackage(
  NAME rpmalloc
  VERSION 2.0.0
  GITHUB_REPOSITORY mjansson/rpmalloc
  GIT_TAG 2.0.0
  DOWNLOAD_ONLY
)

add_library(rpmalloc ${rpmalloc_SOURCE_DIR}/rpmalloc/rpmalloc.c)
target_include_directories(rpmalloc PUBLIC ${rpmalloc_SOURCE_DIR}/rpmalloc/)

add_library(yyjson deps/yyjson/yyjson.c)
target_include_directories(yyjson PUBLIC deps/yyjson/)

include(deps/engine.cmake)
include(deps/excel.cmake)
