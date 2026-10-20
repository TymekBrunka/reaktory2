#include <LuaBindings.hpp>
#include <Scene.hpp>
#include <iostream>

ScriptClass::~ScriptClass() {
  if (tableRef != -1) {
    // lua_unlock(luas[luaIdx], tableRef);
    luaL_unref(L, LUA_REGISTRYINDEX, tableRef);
  }
}

ScriptClass::ScriptClass(const ScriptClass &other) {
  L = other.L;
  tableRef = other.tableRef;
}

ScriptClass &ScriptClass::operator=(const ScriptClass &other) {
  if (this != &other) {
    L = other.L;
    tableRef = other.tableRef;
  }
  return *this;
}

ScriptClass::ScriptClass(ScriptClass &&other) {
  L = other.L;
  tableRef = other.tableRef;
  other.L = 0;
  other.tableRef = -1;
}

ScriptClass &ScriptClass::operator=(ScriptClass &&other) {
  if (this != &other) {
    L = other.L;
    tableRef = other.tableRef;
    other.L = 0;
    other.tableRef = -1;
  }
  return *this;
}

class SceneLuaBindings {
public:
  static int add_script_class(lua_State *L) {
    int tableRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_rawgeti(L, LUA_REGISTRYINDEX, tableRef);
    if (lua_getfield(L, -1, "name") != LUA_TSTRING) {
      luaL_typeerror(L, -1, "string");
      lua_pop(L, -1);
      lua_pop(L, -1);
      return 0;
    }

    const char *name = lua_tostring(L, -1);

    Scene *scene = *(Scene **)lua_touserdata(L, lua_upvalueindex(1));
    ScriptClass sc{L, tableRef, name};
    scene->script_classes.push_back(sc);

    lua_rawgeti(L, LUA_REGISTRYINDEX, sc.tableRef);
    lua_getfield(L, -1, "start");
    call_lua(L);

    return 0;
  }

  // static int object_get_name(lua_State *L) {
  //   return 0;
  // }
};

void Scene::add_lua(lua_State *L) {
  luas.push_back(L);

  lua_newtable(L);

  lua_pushlightuserdata(L, self);
  lua_pushcclosure(L, SceneLuaBindings::add_script_class, 1);
  lua_setfield(L, -2, "register_class");
  lua_setglobal(L, "reaktory");

  luaL_newmetatable(L, "Object pool");

  Scene **ud = (Scene **)lua_newuserdata(L, sizeof(void *));
  *ud = *self;

  std::cerr << "lua type: " << (int)(lua_type(L, -3) - LUA_TNIL) << "\n";
  lua_getfield(L, -3, "register_class");
  std::cerr << "lua type: " << (int)(lua_type(L, -1) - LUA_TNIL) << "\n";
  // lua_setfield(L, -3, "pool");
  lua_setfield(L, -4, "pool");
}
