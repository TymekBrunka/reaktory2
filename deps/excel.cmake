#excel
set(CPM_USE_LOCAL_PACKAGES OFF)
message(expat)
CPMAddPackage(
  NAME expat
  VERSION 2.7.4
  GITHUB_REPOSITORY libexpat/libexpat
  GIT_TAG R_2_7_4
  DOWNLOAD_ONLY
)
# set(CPM_USE_LOCAL_PACKAGES ON)

set(EXPAT_BUILD_TOOLS OFF)
set(EXPAT_SHARED_LIBS OFF)
set(EXPAT_BUILD_EXAMPLES OFF)
set(EXPAT_BUILD_TESTS OFF)
set(EXPAT_ENABLE_INSTALL OFF)

set(EXPAT_DIR "EXPAT_DIR ${expat_SOURCE_DIR}/expat")

message(STATUS expatsrc ${expat_SOURCE_DIR})
add_subdirectory(${expat_SOURCE_DIR}/expat ${CMAKE_BINARY_DIR}/expat.dir)

get_target_property(EXPAT_INCLUDES expat INCLUDE_DIRECTORIES)
set(EXPAT_INCLUDE_DIR ${zlibincludes} CACHE PATH "" FORCE)

# CPMAddPackage(
#   NAME libarchive
#   VERSION 3.8.7
#   GITHUB_REPOSITORY libarchive/libarchive
#   GIT_TAG v3.8.7
#   OPTIONS
#     "BUILD_SHARED_LIBS OFF"
#     "ENABLE_INSTALL OFF"
#     "ENABLE_TEST OFF"
#     "ENABLE_BZip2 OFF"
#     "ENABLE_LIBXML2 OFF"
#     "ENABLE_EXPAT OFF"
# )
#
# target_link_libraries(archive_static zlibstatic)

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
# add_library(zstd::libzstd_static ALIAS libzstd_static)

message(libzip)
CPMAddPackage(
  NAME libzip
  VERSION 1.11.4
  GITHUB_REPOSITORY nih-at/libzip
  GIT_TAG v1.11.4
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

get_target_property(LIBZIP_INCLUDES libzip::zip INCLUDE_DIRECTORIES)
list(APPEND LIBZIP_INCLUDES $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps> $<BUILD_INTERFACE:${libzip_BINARY_DIR}>)

message(xlsxio)
CPMAddPackage(
  NAME xlsxio
  VERSION 0.2.36
  GITHUB_REPOSITORY brechtsanders/xlsxio
  GIT_TAG 0.2.36
  OPTIONS
    "CMAKE_POLICY_VERSION_MINIMUM 3.5"
    "BUILD_STATIC ON"
    "BUILD_SHARED OFF"
    "BUILD_TOOLS OFF"
    "BUILD_DOCUMENTATION OFF"
    "BUILD_PC_FILES OFF"
    "BUILD_EXAMPLES OFF"
    # "WITH_MINIZIP_NG ON"
    "WITH_LIBZIP ON"

    "EXPAT_INCLUDE_DIR ${EXPAT_INCLUDES}"
    "EXPAT_LIBRARIES expat"
    "EXPAT_DIR ${EXPAT_DIR}"
    "ZLIB_LIBRARY "
    "ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIRS}"
    # "ZLIB_DIR ${zlib_SOURCE_DIR}"

    "LIBZIP_INCLUDE_DIRS ${LIBZIP_INCLUDES}"
)

target_link_libraries(xlsxio_read_STATIC zip)
target_include_directories(xlsxio_read_STATIC PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>)

target_link_libraries(xlsxio_write_STATIC zip)
target_include_directories(xlsxio_write_STATIC PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>)
