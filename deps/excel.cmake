#excel
set(CPM_USE_LOCAL_PACKAGES OFF)

message(bzip2)
CPMAddPackage(
  NAME bzip2
  VERSION 1.0
  GIT_REPOSITORY git://sourceware.org/git/bzip2.git
  GIT_TAG f5dfc96063fef2d9c5446d0733867d899ef9b942
  DOWNLOAD_ONLY
)

add_library(bzip2_objs OBJECT
  ${bzip2_SOURCE_DIR}/blocksort.c
  ${bzip2_SOURCE_DIR}/huffman.c
  ${bzip2_SOURCE_DIR}/crctable.c
  ${bzip2_SOURCE_DIR}/randtable.c
  ${bzip2_SOURCE_DIR}/compress.c
  ${bzip2_SOURCE_DIR}/decompress.c
  ${bzip2_SOURCE_DIR}/bzlib.c
)
target_include_directories(bzip2_objs PUBLIC ${bzip2_SOURCE_DIR})
add_library(bzip2 STATIC deps/dummy.c)
target_link_libraries(bzip2 PUBLIC bzip2_objs)

set(cmpath ${CMAKE_MODULE_PATH})
list(PREPEND cmpath "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake-off/")
set(CMAKE_MODULE_PATH ${cmpath} CACHE STRING "" FORCE)

# message(zstd)
# CPMAddPackage(
#   NAME zstd
#   VERSION 0.4.2
#   GITHUB_REPOSITORY facebook/zstd
#   GIT_TAG 5c7b7bad26808e6b40ac3b3d0075466e27738a9d
#   OPTIONS
#     "ZSTD_BUILD_STATIC ON"
#     "ZSTD_BUILD_SHARED ON"
#     "ZSTD_BUILD_PROGRAMS OFF"
#     "BUILD_TESTING OFF"
# )

set(ZSTD_FOUND TRUE CACHE BOOL "" FORCE)
set(BZIP2_FOUND TRUE CACHE BOOL "" FORCE)
# add_library(zstd::libzstd_static ALIAS libzstd_static)
add_library(BZip2::BZip2 ALIAS bzip2)

message(libzip)
CPMAddPackage(
  NAME libzip
  VERSION 1.11.3
  GITHUB_REPOSITORY nih-at/libzip
  GIT_TAG v1.11.3
  OPTIONS
    "ENABLE_LZMA ON"
    "ENABLE_ZSTD ON"
    "ENABLE_OPENSSL OFF"
    "ENABLE_COVERAGE OFF"
    "ENABLE_COMMONCRYPTO OFF"
    "ENABLE_GNUTLS OFF"
    "ENABLE_MBEDTLS OFF"
    "ENABLE_BZIP2 ON"
    "ENABLE_FDOPEN OFF"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_TOOLS OFF"
    "BUILD_DOC OFF"
    "BUILD_EXAMPLES OFF"
    "BUILD_OSSFUZZ OFF"
    "BUILD_REGRESS OFF"
    "LIBZIP_DO_INSTALL OFF"
    "BUILD_SHARED_LIBS OFF"

    # "CFLAGS -I${libzip_SOURCE_DIR}"
)

# target_include_directories(zip PUBLIC ${libzip_SOURCE_DIR})
target_link_libraries(zip PRIVATE zlibstatic)

set(CMAKE_TLS_VERIFY 0)
set(LibZip_FOUND TRUE CACHE BOOL "" FORCE)

message(openXLSX)
CPMAddPackage(
  NAME OpenXLSX
  VERSION 0.5.2
  GIT_REPOSITORY https://codeberg.org/lars_uffmann/OpenXLSX.git
  GIT_TAG 5888e9441b
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "OPENXLSX_CREATE_DOCS OFF"
    "OPENXLSX_BUILD_SAMPLES OFF"
    "OPENXLSX_ENABLE_LIBZIP ON"
    # "OPENXLSX_MONOLITHIC_LIBRARY ON"
)
