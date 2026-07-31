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

message(bzip2)
CPMAddPackage(
  NAME bzip2
  VERSION 1.0
  GIT_REPOSITORY git://sourceware.org/git/bzip2.git
  GIT_TAG master
)

set(cmpath ${CMAKE_MODULE_PATH})
list(PREPEND cmpath "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake-off/")
set(CMAKE_MODULE_PATH ${cmpath} CACHE ARRAY "" FORCE)

message(zstd)
CPMAddPackage(
  NAME zstd
  VERSION 0.4.2
  GITHUB_REPOSITORY facebook/zstd
  GIT_TAG 5c7b7bad26808e6b40ac3b3d0075466e27738a9d
  OPTIONS
    "ZSTD_BUILD_STATIC ON"
    "ZSTD_BUILD_SHARED ON"
    "ZSTD_BUILD_PROGRAMS OFF"
    "BUILD_TESTING OFF"
)

set(ZSTD_FOUND TRUE CACHE BOOL "" FORCE)
add_library(zstd::libzstd_static ALIAS libzstd_static)

message(zlib)

if(WIN32)
  set(zlib_static_suffix "s")
endif()

set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE) # the long way since minizpi is built inside zlib cmake script so there is no access to zlibstatic target yet
add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}z${zlib_static_suffix}${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE PATH "" FORCE)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE ARRAY "" FORCE)
set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_LOCATION "${ZLIB_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${zlib_INCLUDE_DIR}")

CPMAddPackage(
  NAME zlib
  VERSION 1.3.1.2
  GITHUB_REPOSITORY madler/zlib
  GIT_TAG v1.3.1.2
  OPTIONS
    "ZLIB_BUILD_SHARED OFF"
    "ZLIB_BUILD_STATIC ON"
    "ZLIB_BUILD_MINIZIP ON"
    "ZLIB_BUILD_TESTING OFF"
    "ZLIB_INSTALL OFF"

    "ZLIB_BUILD_MINIZIP ON"
    "MINIZIP_BUILD_SHARED OFF"
    "MINIZIP_INSTALL OFF"
    "MINIZIP_BUILD_TESTING OFF"
)

# add_library(ZLIB::ZLIB ALIAS zlibstatic)
# set(minizip_FOUND TRUE CACHE BOOL "" FORCE)
# add_library(minizip::minizip ALIAS libminizipstatic)

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
