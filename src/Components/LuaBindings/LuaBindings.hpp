#pragma once
#include <lua.hpp>
#include <string>
#include <unordered_map>

static constexpr int TRACELOG_IDX = 4;

#define LUAB_CARGCHECK(argc)                                                   \
  if (lua_gettop(L) != (argc)) {                                               \
    luaL_error(L, "invalid number of arguments (%d expected, got %d)", (argc), \
               lua_gettop(L));                                                 \
    return 0;                                                                  \
  }

#define LUAB_MCARGCHECK(argc)                                                  \
  if (lua_gettop(L) != (argc) + 1) {                                           \
    luaL_error(L, "invalid number of arguments (%d expected, got %d)", (argc), \
               lua_gettop(L) - 1);                                             \
  }

#define LUAB_METHODCHECK(type, typename_)                                      \
  (type *)luaL_testudata(L, 1, typename_);                                     \
  if (!ud) {                                                                   \
    luaL_argerror(L, 1, typename_);                                            \
    return 0;                                                                  \
  }

#define LUAB_ARGCHECK(arg, type, name, typename_)                              \
  type *name = (type *)luaL_testudata(L, (arg), typename_);                    \
  if (!name) {                                                                 \
    luaL_argerror(L, (arg), typename_);                                        \
    return 0;                                                                  \
  }

const char *get_lua_typename(lua_State *L, int idx);

void init_lua(lua_State **L);

bool call_lua(lua_State *L, int argc = 0);

void close_lua(lua_State *L);

struct string_hash {
  using is_transparent = void;
  [[nodiscard]] inline size_t operator()(const char *txt) const {
    return std::hash<std::string_view>{}(txt);
  }
  [[nodiscard]] inline size_t operator()(std::string_view txt) const {
    return std::hash<std::string_view>{}(txt);
  }
  [[nodiscard]] inline size_t operator()(const std::string &txt) const {
    return std::hash<std::string>{}(txt);
  }
};

extern std::unordered_map<std::string, std::string, string_hash,
                          std::equal_to<>>
    lua_script_modules_;

decltype(lua_script_modules_) &lua_script_modules();
