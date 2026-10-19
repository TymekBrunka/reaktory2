#include <LuaBindings.hpp>
#include <iostream>
#include <lua_cpml.h>

std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>
    lua_script_modules_;

static int embeded_scripts_searcher(lua_State *L);
static int embeded_scripts_loader(lua_State *L);

void init_lua(lua_State **L_) {
  lua_State *&L = *L_;
  L = luaL_newstate();
  // luaL_openlibs(L);
  luaL_requiref(L, LUA_GNAME, luaopen_base, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_LOADLIBNAME, luaopen_package, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, 1);
  lua_pop(L, 1);
  luaL_requiref(L, LUA_DBLIBNAME, luaopen_debug, 1);
  lua_pop(L, 1);

  lua_getglobal(L, LUA_LOADLIBNAME);
  if (!lua_istable(L, -1))
    return;

  lua_getfield(L, -1, "searchers");
  if (!lua_istable(L, -1))
    return;

  lua_pushvalue(L, -2);
  lua_pushcclosure(L, embeded_scripts_searcher, 1);
  lua_rawseti(L, -2, 5);
  lua_setfield(L, -2, "searchers");

  lua_script_modules();

  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_remove(L, -2);

  lua_insert(L, 1);
}

bool call_lua(lua_State *L) {
  if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
    std::cerr << "luerror: " << lua_tostring(L, -1) << "\n";
    lua_pop(L, 1);
    return false;
  }
  return true;
}

void close_lua(lua_State *L) {
  lua_remove(L, 1);
  lua_close(L);
}

decltype(lua_script_modules_) &lua_script_modules() {
  if (lua_script_modules_.empty()) {
    std::cerr << "Initialising some scripts\n";
    lua_script_modules_["cpml.init"] = (const char *)init_lua_data;
    lua_script_modules_["cpml.modules.color"] = (const char *)color_lua_data;
    lua_script_modules_["cpml.modules.constants"] =
        (const char *)constants_lua_data;
    lua_script_modules_["cpml.modules.intersect"] =
        (const char *)intersect_lua_data;
    lua_script_modules_["cpml.modules.mat4"] = (const char *)mat4_lua_data;
    lua_script_modules_["cpml.modules.mesh"] = (const char *)mesh_lua_data;
    lua_script_modules_["cpml.modules.octree"] = (const char *)octree_lua_data;
    lua_script_modules_["cpml.modules.quat"] = (const char *)quat_lua_data;
    lua_script_modules_["cpml.modules.simplex"] =
        (const char *)simplex_lua_data;
    lua_script_modules_["cpml.modules.utils"] = (const char *)utils_lua_data;
    lua_script_modules_["cpml.modules.vec2"] = (const char *)vec2_lua_data;
    lua_script_modules_["cpml.modules.vec3"] = (const char *)vec3_lua_data;
  }
  return lua_script_modules_;
}

static int embeded_scripts_searcher(lua_State *L) {
  const char *path_ = lua_tostring(L, -1);
  std::string path = path_;

  auto script = lua_script_modules_.find(path);
  if (script == lua_script_modules_.end()) {
    std::string s = path;
    s += ".init";
    script = lua_script_modules_.find(s);
  }

  if (script != lua_script_modules_.end()) {
    lua_pushcclosure(L, embeded_scripts_loader, 1);
    lua_pushstring(L, path_);
    return 2;
  }

  lua_pushstring(L, "not found");
  return 1;
}

static int embeded_scripts_loader(lua_State *L) {
  const char *path = lua_tostring(L, -1);

  auto script = lua_script_modules_.find(path);
  if (script == lua_script_modules_.end()) {
    std::string s = path;
    s += ".init";
    script = lua_script_modules_.find(s);
  }

  luaL_loadstring(L, script->second.c_str());
  lua_pushstring(L, path);

  if (lua_pcall(L, 1, LUA_MULTRET, 1) != LUA_OK) {
    std::cerr << "luerror: " << lua_tostring(L, -1) << "\n";
    lua_pop(L, 1);
  }

  return 1;
}
