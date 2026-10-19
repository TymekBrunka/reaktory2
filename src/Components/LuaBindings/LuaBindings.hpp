#pragma once
#include <lua.hpp>
#include <string>
#include <unordered_map>

void init_lua(lua_State** L);

bool call_lua(lua_State *L);

void close_lua(lua_State* L);

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

extern std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>
    lua_script_modules_;

decltype(lua_script_modules_) &lua_script_modules();
