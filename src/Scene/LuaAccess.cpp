#include "ObjectPool.hpp"
#include <LuaBindings.hpp>
#include <Scene.hpp>
#include <iostream>

void ScriptClass::destroy() {
  if (tableidx != -1) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_pushnil(L);
    lua_rawseti(L, -2, tableidx + 1);
    (*scene)->script_class_free_list.push_back(
        Scene::tabidx{.tabidx = tableidx, .L = L});
  }
}

ScriptClass::ScriptClass(const ScriptClass &other) {
  L = other.L;
  tableidx = other.tableidx;
}

ScriptClass &ScriptClass::operator=(const ScriptClass &other) {
  if (this != &other) {
    L = other.L;
    tableidx = other.tableidx;
  }
  return *this;
}

ScriptClass::ScriptClass(ScriptClass &&other) {
  L = other.L;
  tableidx = other.tableidx;
  other.L = nullptr;
  other.tableidx = -1;
}

ScriptClass &ScriptClass::operator=(ScriptClass &&other) {
  if (this != &other) {
    L = other.L;
    tableidx = other.tableidx;
    other.L = nullptr;
    other.tableidx = -1;
  }
  return *this;
}

class SceneLuaBindings {
public:
  static int scene_get_name(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    lua_pushstring(L, (*(*ud))->name.c_str());
    return 1;
  }

  static int scene_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    lua_pushfstring(L, "Scene {name = %s, objects = %d}",
                    (*(*ud))->name.c_str(), (*(*ud))->objPool.size());
    return 1;
  }

  static int add_script_class(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    Scene *scene = *(*ud);
    int tableidx = -1;
    if (!scene->script_class_free_list.empty()) {
      int i = scene->script_class_free_list.size() - 1;
      for (auto freeidx = scene->script_class_free_list.rbegin();
           freeidx != scene->script_class_free_list.rend(); freeidx++) {
        if (freeidx->L == L) {
          tableidx = freeidx->tabidx;
          scene->script_class_free_list.erase(
              scene->script_class_free_list.begin() + i);
          break;
        }
        i--;
      }
    } else {
      tableidx = scene->script_classes.size();
    }

    if (lua_getfield(L, 2, "name") != LUA_TSTRING) {
      luaL_error(
          L,
          "bad 'name' field type in argument #1 to 'register_class' (string "
          "expected, got %s)",
          get_lua_typename(L, -1));
      return 0;
    }

    const char *name = lua_tostring(L, -1);

    ScriptClass sc{&scene, L, tableidx, name};
    lua_remove(L, -1); // remove name copy from stack
    scene->script_classes.push_back(sc);

    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, tableidx + 1);
    lua_remove(L, -1);

    // lua_pushvalue(L, 2);
    lua_getfield(L, 2, "start");
    call_lua(L);

    return 0;
  }

  static int objH_get_gen(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    lua_pushinteger(L, ud->gen);
    return 1;
  }

  static int objH_get_idx(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    lua_pushinteger(L, ud->idx);
    return 1;
  }

  static int objH_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    lua_pushfstring(L, "ObjectHandle {gen = %d, idx = %d}", (int)ud->gen,
                    (int)ud->idx);
    return 1;
  }

  static int pool_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    lua_pushfstring(L, "ObjectPool {size=%d, capacity=%d}",
                    (int)(*(*ud))->objPool.size(),
                    (int)(*(*ud))->objPool.real_size());
    return 1;
  }

  static int pool_get(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene *, "ObjectPool");
    LUAB_ARGCHECK(2, objH, h, "ObjectHandle");
    Object *obj = (*ud)->objPool.get(*h);
    if (!obj) {
      lua_pushnil(L);
      return 1;
    }
    Object **ret = (Object **)lua_newuserdata(L, sizeof(Object *));
    *ret = obj;
    luaL_getmetatable(L, "ObjectRef");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int pool_get_root(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    objH *obj = (objH *)lua_newuserdata(L, sizeof(objH));
    *obj = (*(*ud))->objPool.rootH();
    luaL_getmetatable(L, "ObjectHandle");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int pool_remove(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    LUAB_ARGCHECK(2, objH, obj, "ObjectHandle");
    (*(*ud))->objPool.remove(*obj);
    return 0;
  }

  static int obj_get_name(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    lua_pushstring(L, (*ud)->name.c_str());
    return 1;
  }

  static int obj_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    const char *type = "[unknown]";
    if (std::get_if<oEmptySlot>(&(*ud)->variant))
      type = "[empty slot]";
    else if (std::get_if<oNode>(&(*ud)->variant))
      type = "[node]";
    else if (std::get_if<oModel>(&(*ud)->variant))
      type = "[model]";
    lua_pushfstring(L,
                    "ObjectRef {name = %s, type = %s, parent = {gen = %d, idx = "
                    "%d}, numChildren = %d}",
                    (*ud)->name.empty() ? "" : (*ud)->name.c_str(), type,
                    (int)(*ud)->parent.gen, (int)(*ud)->parent.idx,
                    (*ud)->children.size());
    return 1;
  }
};

void Scene::add_lua(lua_State *L) {
  luas.push_back(L);

  lua_newtable(L);
  int scene_script_classes_idx =
      luaL_ref(L, LUA_REGISTRYINDEX); // not the global constexpr
  std::cerr << "scene script classes idx is " << scene_script_classes_idx
            << "\n";

  // scene
  {
    Scene ***ud = (Scene ***)lua_newuserdata(L, sizeof(void *));
    *ud = self;
    luaL_newmetatable(L, "Scene");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */

    luaL_Reg funcs[] = {
        {"name", SceneLuaBindings::scene_get_name},
        {"register_class", SceneLuaBindings::add_script_class},
        {"__tostring", SceneLuaBindings::scene_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_setmetatable(L, -2); // 1st - udata, 2nd - (meta)table
  }

  lua_pushvalue(L, -1);
  lua_setglobal(L, "scene");
  lua_getmetatable(L, -1); // push scene's metadata for appending fields to

  // object handle
  {
    luaL_newmetatable(L, "ObjectHandle");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"gen", SceneLuaBindings::objH_get_gen},
        {"idx", SceneLuaBindings::objH_get_idx},
        {"__tostring", SceneLuaBindings::objH_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_remove(L, -1);
  }

  // object reference
  {
    luaL_newmetatable(L, "ObjectRef");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"name", SceneLuaBindings::obj_get_name},
        {"__tostring", SceneLuaBindings::obj_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_remove(L, -1);
  }

  // object pool
  {
    Scene ***ud = (Scene ***)lua_newuserdata(L, sizeof(void *));
    *ud = self;
    luaL_newmetatable(L, "ObjectPool");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */

    luaL_Reg funcs[] = {
        {"get", SceneLuaBindings::pool_get},
        {"root", SceneLuaBindings::pool_get_root},
        {"remove", SceneLuaBindings::pool_remove},
        {"__tostring", SceneLuaBindings::pool_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_setmetatable(L, -2); // 1st - udata, 2nd - (meta)table
  }

  lua_setfield(L, -2, "pool");
  // lua_remove(L, -1);
  // lua_remove(L, -1);

  // for (int i = 1; i < 7; i++)
  //   std::cerr << "lua type: " << (int)(lua_type(L, i) - LUA_TNIL) << "\n";
}
