cmake_minimum_required(VERSION 3.23)
project(lua_cmake LANGUAGES C)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  if (CMAKE_C_COMPILER_ID MATCHES "MSVC") # not sure if this project will build on msvc but regardless, ill set this flags for it just in case

    if(NOT CMAKE_C_FLAGS_RELEASE)
      set(CMAKE_C_FLAGS_RELEASE "/O2 /std:c99")
    endif()

  else()

    if(NOT CMAKE_C_FLAGS_RELEASE)
      set(CMAKE_C_FLAGS_RELEASE "-O2 -std=gnu99")
    endif()

  endif()
endif()

add_library(lua_core OBJECT
 lua/src/lapi.c lua/src/lcode.c lua/src/lctype.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c
)

add_library(lua_lib OBJECT
 lua/src/lauxlib.c lua/src/lbaselib.c lua/src/lcorolib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loadlib.c lua/src/loslib.c lua/src/lstrlib.c lua/src/ltablib.c lua/src/lutf8lib.c lua/src/linit.c
)

add_library(lua STATIC dummy.c)
target_include_directories(lua PUBLIC lua/src)
target_link_libraries(lua PUBLIC lua_lib lua_core)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  add_executable(test test/test.c)
  target_link_libraries(test PRIVATE lua)
endif()
