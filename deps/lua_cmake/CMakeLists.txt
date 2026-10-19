cmake_minimum_required(VERSION 3.23)
project(lua_cmake LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_library(lua_core OBJECT
 lua/src/lapi.c lua/src/lcode.c lua/src/lctype.c lua/src/ldebug.c lua/src/ldo.c lua/src/ldump.c lua/src/lfunc.c lua/src/lgc.c lua/src/llex.c lua/src/lmem.c lua/src/lobject.c lua/src/lopcodes.c lua/src/lparser.c lua/src/lstate.c lua/src/lstring.c lua/src/ltable.c lua/src/ltm.c lua/src/lundump.c lua/src/lvm.c lua/src/lzio.c
)
target_include_directories(lua_core PRIVATE lua/src)
target_compile_definitions(lua_core PRIVATE LUA_COMPAT_5_3)

add_library(lua_lib OBJECT
 lua/src/lauxlib.c lua/src/lbaselib.c lua/src/lcorolib.c lua/src/ldblib.c lua/src/liolib.c lua/src/lmathlib.c lua/src/loadlib.c lua/src/loslib.c lua/src/lstrlib.c lua/src/ltablib.c lua/src/lutf8lib.c lua/src/linit.c
)
target_include_directories(lua_lib PRIVATE lua/src)
target_compile_definitions(lua_lib PRIVATE LUA_COMPAT_5_3)

# add_library(lua STATIC $<TARGET_OBJECTS:lua_core> $<TARGET_OBJECTS:lua_lib>)
add_library(lua STATIC dummy.c)
target_link_libraries(lua PRIVATE lua_core lua_lib)
target_include_directories(lua PUBLIC lua/src)

target_compile_definitions(lua_core PRIVATE LUA_COMPAT_5_3)
target_compile_definitions(lua_lib PRIVATE LUA_COMPAT_5_3)
if (NOT CMAKE_C_COMPILER_ID MATCHES "MSVC")
  target_link_libraries(lua PRIVATE m)
endif()

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  add_executable(test test/test.c)
  target_link_libraries(test PRIVATE lua)
endif()
