#include <LuaBindings.hpp>
#include <cstring>
#include <iostream>
#include <lua_cpml.h>

std::unordered_map<std::string, std::string, string_hash, std::equal_to<>>
    lua_script_modules_;

static int embeded_scripts_searcher(lua_State *L);
static int embeded_scripts_loader(lua_State *L);

const char *get_lua_typename(lua_State *L, int idx) {
  const char *typearg; /* name for the type of the actual argument */
  if (luaL_getmetafield(L, idx, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1); /* use the given type name */
  else if (lua_type(L, idx) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata"; /* special name for messages */
  else
    typearg = luaL_typename(L, idx); /* standard name */

  return typearg;
}

int push_lua_error(lua_State *L, const char *fmt, ...) {
  lua_pushnil(L);
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  return 0;
}

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

  int tracelogidx = luaL_ref(L, LUA_REGISTRYINDEX); // not the global constexpr
  std::cerr << "tracelogidx is " << tracelogidx << "\n";

  // lua_insert(L, 1); // traceback function at index 1
}

bool call_lua(lua_State *L, int argc) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, tracelogIdx);
  lua_insert(L, -2);
  int tracelog_idx = lua_gettop(L) -1;
  if (lua_pcall(L, argc, LUA_MULTRET, tracelog_idx) != LUA_OK) {
    std::cerr << "\x1b[31m" << lua_tostring(L, -1) << "\x1b[0m\n";
    lua_pop(L, 1);
    return false;
  }
  lua_remove(L, tracelog_idx);
  return true;
}

void close_lua(lua_State *L) {
  luaL_unref(L, LUA_REGISTRYINDEX, /*tracelogidx*/ tracelogIdx);
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

  // luaL_loadstring(L, script->second.c_str());
  lua_load(
      L,
      [](lua_State *L, void *data, size_t *size) {
        *size = strlen((const char *)data);
        return (const char *)data;
      },
      (char *)script->second.c_str(), path, "bt");
  lua_pushstring(L, path);

  if (lua_pcall(L, 1, LUA_MULTRET, 1) != LUA_OK) {
    std::cerr << "luerror: " << lua_tostring(L, -1) << "\n";
    lua_pop(L, 1);
  }

  return 1;
}
