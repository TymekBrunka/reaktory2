#include "ObjectPool.hpp"
#include "glm/ext/vector_float4.hpp"
#include <LuaBindings.hpp>
#include <Scene.hpp>
#include <cstring> // for memset bc cppussy cpp tries to destroy object that's not yet created (junk memory)
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
  scene = other.scene;
  name = other.name;
}

ScriptClass &ScriptClass::operator=(const ScriptClass &other) {
  if (this != &other) {
    L = other.L;
    tableidx = other.tableidx;
    scene = other.scene;
    name = other.name;
  }
  return *this;
}

ScriptClass::ScriptClass(ScriptClass &&other) {
  L = other.L;
  tableidx = other.tableidx;
  name = std::move(other.name);
  other.L = nullptr;
  other.tableidx = -1;
  Scene **tmp = scene;
  scene = other.scene;
  other.scene = tmp;
}

ScriptClass &ScriptClass::operator=(ScriptClass &&other) {
  if (this != &other) {
    L = other.L;
    tableidx = other.tableidx;
    name = std::move(other.name);
    other.L = nullptr;
    other.tableidx = -1;
    Scene **tmp = scene;
    scene = other.scene;
    other.scene = tmp;
  }
  return *this;
}

class SceneLuaBindings {
public:
  static int scene_get_name(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    LUAB_MCARGCHECK(0);
    lua_pushstring(L, (*(*ud))->name.c_str());
    return 1;
  }

  static int scene_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(L, "Scene {name = %s, objects = %d}",
                    (*(*ud))->name.c_str(), (*(*ud))->objPool.size());
    return 1;
  }

  static int add_script_class(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "Scene");
    LUAB_MCARGCHECK(1);
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
    scene->script_classes.push_back(std::move(sc));

    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, tableidx + 1);
    lua_remove(L, -1);

    int ftype = lua_getfield(L, 2, "on_attach");
    if (ftype != LUA_TNIL && ftype != LUA_TFUNCTION) {
      luaL_error(
          L,
          "bad 'on_attach' field type in argument #1 to 'register_class' "
          "(function or nil"
          " expected, got %s)",
          get_lua_typename(L, -1));
      return 0;
    }
    lua_remove(L, -1); // remove name copy from stack

    ftype = lua_getfield(L, 2, "on_detach");
    if (ftype != LUA_TNIL && ftype != LUA_TFUNCTION) {
      luaL_error(
          L,
          "bad 'on_detach' field type in argument #1 to 'register_class' "
          "(function or nil"
          " expected, got %s)",
          get_lua_typename(L, -1));
      return 0;
    }
    lua_remove(L, -1); // remove name copy from stack

    ftype = lua_getfield(L, 2, "on_update");
    if (ftype != LUA_TNIL && ftype != LUA_TFUNCTION) {
      luaL_error(
          L,
          "bad 'on_update' field type in argument #1 to 'register_class' "
          "(function or nil"
          " expected, got %s)",
          get_lua_typename(L, -1));
      return 0;
    }
    lua_remove(L, -1); // remove name copy from stack

    ftype = lua_getfield(L, 2, "on_frame");
    if (ftype != LUA_TNIL && ftype != LUA_TFUNCTION) {
      luaL_error(L,
                 "bad 'on_frame' field type in argument #1 to 'register_class' "
                 "(function or nil"
                 " expected, got %s)",
                 get_lua_typename(L, -1));
      return 0;
    }
    lua_remove(L, -1); // remove name copy from stack

    // lua_getfield(L, 2, "on_start");
    // call_lua(L);

    return 0;
  }

  static int objH_new(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    LUAB_MCARGCHECK(2);
    int isnum;
    auto gen = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    auto idx = lua_tointegerx(L, 3, &isnum);
    if (!isnum) {
      luaL_argerror(L, 3, "number has no integer representation");
      return 0;
    }
    objH *obj = (objH *)lua_newuserdata(L, sizeof(objH));
    *obj = objH{.gen = (uint8_t)gen, .idx = (int16_t)idx};
    luaL_getmetatable(L, "ObjectHandle");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int objH_get_gen(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    LUAB_MCARGCHECK(0);
    lua_pushinteger(L, ud->gen);
    return 1;
  }

  static int objH_get_idx(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    LUAB_MCARGCHECK(0);
    lua_pushinteger(L, ud->idx);
    return 1;
  }

  static int objH_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(objH, "ObjectHandle");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(L, "ObjectHandle {gen = %d, idx = %d}", (int)ud->gen,
                    (int)ud->idx);
    return 1;
  }

  static int pool_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(L, "ObjectPool {size=%d, capacity=%d}",
                    (int)(*(*ud))->objPool.size(),
                    (int)(*(*ud))->objPool.real_size());
    return 1;
  }

  static int pool_nth_child_of(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    LUAB_MCARGCHECK(2);
    LUAB_ARGCHECK(2, Object *, obj, "ObjectRef");
    int isnum;
    auto childidx = lua_tointegerx(L, 3, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    Object *child = (*(*ud))->objPool.getChildOf(*obj, (int16_t)childidx - 1);
    if (!child) {
      lua_pushnil(L);
      return 1;
    }
    Object **ret = (Object **)lua_newuserdata(L, sizeof(Object *));
    *ret = child;
    luaL_getmetatable(L, "ObjectRef");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int pool_get(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    LUAB_MCARGCHECK(1);
    LUAB_ARGCHECK(2, objH, h, "ObjectHandle");
    // std::cerr << "obj handle is {" << h->gen << ", " << h->idx << "}\n";
    Object *obj = (*(*ud))->objPool.get(*h);
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
    LUAB_MCARGCHECK(0);
    objH *obj = (objH *)lua_newuserdata(L, sizeof(objH));
    *obj = (*(*ud))->objPool.rootH();
    luaL_getmetatable(L, "ObjectHandle");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int pool_remove(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Scene **, "ObjectPool");
    LUAB_MCARGCHECK(1);
    LUAB_ARGCHECK(2, objH, obj, "ObjectHandle");
    (*(*ud))->objPool.remove(*obj);
    return 0;
  }

  static int obj_get_name(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    lua_pushstring(L, (*ud)->name.c_str());
    return 1;
  }

  static int obj_set_name(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(1);
    const char *name = lua_tostring(L, 2);
    if (!name) {
      luaL_argerror(L, 2, "string");
      return 0;
    }
    (*ud)->name = name;
    return 0;
  }

  static int obj_get_variant(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    int variant = -1;
    if (std::get_if<oEmptySlot>(&(*ud)->variant))
      variant = 0;
    else if (std::get_if<oNode>(&(*ud)->variant))
      variant = 1;
    else if (std::get_if<oModel>(&(*ud)->variant))
      variant = 2;
    lua_pushnumber(L, variant);
    return 1;
  }

  static int obj_get_parent(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    objH *obj = (objH *)lua_newuserdata(L, sizeof(objH));
    *obj = (*ud)->parent;
    luaL_getmetatable(L, "ObjectHandle");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int obj_set_parent(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(1);
    LUAB_ARGCHECK(2, objH, parent, "ObjectHandle");
    (*ud)->parent = *parent;
    return 1;
  }

  static int obj_get_numchildren(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, (*ud)->children.size());
    return 1;
  }

  static int obj_to_handle(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto idx = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    objH *obj = (objH *)lua_newuserdata(L, sizeof(objH));
    *obj = (*ud)->toHandle((int16_t)idx);
    luaL_getmetatable(L, "ObjectHandle");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int obj_switch_variant(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto variant = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    switch (variant) {
    case 0:
      (*ud)->variant = oNode{};
      return 0;
    case 1:
      (*ud)->variant = oModel{};
      return 0;
    default:
      luaL_error(L, "invalid object variant index %d", (int)variant);
      return 0;
    }
  }

  static int obj_try_get_model(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    if (const oModel *omodel = std::get_if<oModel>(&(*ud)->variant)) {
      oModel **omodel_ = (oModel **)lua_newuserdata(L, sizeof(void *));
      *omodel_ = (oModel *)omodel;
      luaL_getmetatable(L, "oModelRef");
      lua_setmetatable(L, -2);
      return 1;
    } else {
      lua_pushnil(L);
      return 1;
    }
  }

  static int obj_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Object *, "ObjectRef");
    LUAB_MCARGCHECK(0);
    const char *type = "[unknown]";
    if (std::get_if<oEmptySlot>(&(*ud)->variant))
      type = "[empty slot]";
    else if (std::get_if<oNode>(&(*ud)->variant))
      type = "[node]";
    else if (std::get_if<oModel>(&(*ud)->variant))
      type = "[model]";
    lua_pushfstring(
        L,
        "ObjectRef {name = %s, variant = %s, parent = {gen = %d, idx = "
        "%d}, numChildren = %d}",
        (*ud)->name.empty() ? "" : (*ud)->name.c_str(), type,
        (int)(*ud)->parent.gen, (int)(*ud)->parent.idx, (*ud)->children.size());
    return 1;
  }

  static int omodel_get_model(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushstring(L, (*ud)->model.c_str());
    return 1;
  }

  static int omodel_set_model(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(1);
    const char *name = lua_tostring(L, 2);
    if (!name) {
      luaL_argerror(L, 2, "string");
      return 0;
    }
    (*ud)->model = name;
    return 0;
  }

  static int omodel_get_animation_idx(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, (*ud)->animation_idx);
    return 1;
  }

  static int omodel_set_animation_idx(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto idx = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    (*ud)->animation_idx = (int)idx;
    return 0;
  }

  static int omodel_get_animation_time(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, (*ud)->animation_time);
    return 1;
  }

  static int omodel_set_animation_time(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto idx = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    (*ud)->animation_time = (float)idx;
    return 0;
  }

  static int omodel_get_animation_speed(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, (*ud)->animation_speed);
    return 1;
  }

  static int omodel_set_animation_speed(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto idx = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    (*ud)->animation_speed = (float)idx;
    return 0;
  }

  static int omodel_get_nummaterials(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, (*ud)->materials.size());
    return 1;
  }

  static int omodel_get_nth_material(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto idx = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    Material *material = (Material *)lua_newuserdata(L, sizeof(Material));
    memset(material, 0, sizeof(Material));
    *material = (*ud)->materials[idx - 1];
    luaL_getmetatable(L, "Material");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int omodel_set_nth_material(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(2);
    int isnum;
    auto idx = lua_tointegerx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number has no integer representation");
      return 0;
    }
    LUAB_ARGCHECK(3, Material, material, "Material");
    (*ud)->materials[idx - 1] = std::move(*material);
    return 0;
  }

  static int omodel_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(oModel *, "oModelRef");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(
        L, "oModelRef {model = %s, numMaterials = %d, animationIdx = %d}",
        (*ud)->model.c_str(), (int)(*ud)->materials.size(),
        (*ud)->animation_idx);
    return 1;
  }

  static int material_get_color(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(0);
    glm::vec4 *vec = (glm::vec4 *)lua_newuserdata(L, sizeof(glm::vec4));
    *vec = ud->color_diffuse;
    luaL_getmetatable(L, "Vec4");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int material_set_color(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(1);
    LUAB_ARGCHECK(2, glm::vec4, vec, "Vec4");
    ud->color_diffuse = *vec;
    return 0;
  }

  static int material_get_texture(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(0);
    lua_pushstring(L, ud->diffuse1.c_str());
    return 1;
  }

  static int material_set_texture(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(1);
    const char *name = lua_tostring(L, 2);
    if (!name) {
      luaL_argerror(L, 2, "string");
      return 0;
    }
    ud->diffuse1 = name;
    return 0;
  }

  static int material_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(L, "Material {color = %d %d %d %d, diffuse = %s}",
                    (int)(ud->color_diffuse.x * 255),
                    (int)(ud->color_diffuse.y * 255),
                    (int)(ud->color_diffuse.z * 255),
                    (int)(ud->color_diffuse.w * 255), ud->diffuse1.c_str());
    return 1;
  }

  static int material_destructor(lua_State *L) {
    auto ud = LUAB_METHODCHECK(Material, "Material");
    LUAB_MCARGCHECK(0);
    (*ud).~Material();
    return 0;
  }

  static int vec4_new(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(4);
    int isnum;
    auto x = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    auto y = lua_tonumberx(L, 3, &isnum);
    if (!isnum) {
      luaL_argerror(L, 3, "number");
      return 0;
    }
    auto z = lua_tonumberx(L, 4, &isnum);
    if (!isnum) {
      luaL_argerror(L, 4, "number");
      return 0;
    }
    auto w = lua_tonumberx(L, 5, &isnum);
    if (!isnum) {
      luaL_argerror(L, 5, "number");
      return 0;
    }
    glm::vec4 *vec = (glm::vec4 *)lua_newuserdata(L, sizeof(glm::vec4));
    *vec = glm::vec4(x, y, z, w);
    luaL_getmetatable(L, "Vec4");
    lua_setmetatable(L, -2);
    return 1;
  }

  static int vec4_get_x(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, ud->x);
    return 1;
  }

  static int vec4_set_x(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto num = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    ud->x = num;
    return 0;
  }

  static int vec4_get_y(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, ud->y);
    return 1;
  }

  static int vec4_set_y(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto num = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    ud->y = num;
    return 0;
  }

  static int vec4_get_z(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, ud->z);
    return 1;
  }

  static int vec4_set_z(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto num = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    ud->z = num;
    return 0;
  }

  static int vec4_get_w(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(0);
    lua_pushnumber(L, ud->w);
    return 1;
  }

  static int vec4_set_w(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(1);
    int isnum;
    auto num = lua_tonumberx(L, 2, &isnum);
    if (!isnum) {
      luaL_argerror(L, 2, "number");
      return 0;
    }
    ud->w = num;
    return 0;
  }

  static int vec4_tostring(lua_State *L) {
    auto ud = LUAB_METHODCHECK(glm::vec4, "Vec4");
    LUAB_MCARGCHECK(0);
    lua_pushfstring(L, "Vec4 {%.2f, %.2f, %.2f, %.2f}", ud->x, ud->y, ud->z,
                    ud->w);
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

  lua_newtable(L);
  const char *variant_enums[] = {/*"EMPTY",*/ "NODE", "MODEL", nullptr};
  int i = 0;
  for (const char **variant = variant_enums; *variant != nullptr; variant++) {
    lua_pushnumber(L, i);
    lua_setfield(L, -2, *variant);
    i++;
  }
  lua_setglobal(L, "ObjectVariants");

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
    objH *ud = (objH *)lua_newuserdata(L, sizeof(objH));
    *ud = {};
    luaL_newmetatable(L, "ObjectHandle");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"new", SceneLuaBindings::objH_new},
        {"gen", SceneLuaBindings::objH_get_gen},
        {"idx", SceneLuaBindings::objH_get_idx},
        {"__tostring", SceneLuaBindings::objH_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_setmetatable(L, -2); // 1st - udata, 2nd - (meta)table
    lua_setglobal(L, "objH");
  }

  // vec4
  {
    glm::vec4 *ud = (glm::vec4 *)lua_newuserdata(L, sizeof(glm::vec4));
    *ud = {};
    luaL_newmetatable(L, "Vec4");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"new", SceneLuaBindings::vec4_new},
        {"x", SceneLuaBindings::vec4_get_x},
        {"setX", SceneLuaBindings::vec4_set_x},
        {"y", SceneLuaBindings::vec4_get_y},
        {"setY", SceneLuaBindings::vec4_set_y},
        {"z", SceneLuaBindings::vec4_get_z},
        {"setZ", SceneLuaBindings::vec4_set_z},
        {"w", SceneLuaBindings::vec4_get_w},
        {"setW", SceneLuaBindings::vec4_set_w},
        {"__tostring", SceneLuaBindings::vec4_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_setmetatable(L, -2); // 1st - udata, 2nd - (meta)table
    lua_setglobal(L, "Vec4");
  }

  // object reference
  {
    luaL_newmetatable(L, "ObjectRef");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"name", SceneLuaBindings::obj_get_name},
        {"setName", SceneLuaBindings::obj_set_name},
        {"parent", SceneLuaBindings::obj_get_parent},
        {"setParent", SceneLuaBindings::obj_set_parent},
        {"toHandle", SceneLuaBindings::obj_to_handle},
        {"variant", SceneLuaBindings::obj_get_variant},
        {"model", SceneLuaBindings::obj_try_get_model},
        {"setVariant", SceneLuaBindings::obj_switch_variant},
        {"numChildren", SceneLuaBindings::obj_get_numchildren},
        {"__tostring", SceneLuaBindings::obj_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_remove(L, -1);
  }

  // oModel
  {
    luaL_newmetatable(L, "oModelRef");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"model", SceneLuaBindings::omodel_get_model},
        {"numMaterials", SceneLuaBindings::omodel_get_nummaterials},
        {"nthMaterial", SceneLuaBindings::omodel_get_nth_material},
        {"setNthMaterial", SceneLuaBindings::omodel_set_nth_material},
        {"setModel", SceneLuaBindings::omodel_set_model},
        {"animationIdx", SceneLuaBindings::omodel_get_animation_idx},
        {"setAnimationIdx", SceneLuaBindings::omodel_set_animation_idx},
        {"animationTime", SceneLuaBindings::omodel_get_animation_time},
        {"setAnimationTime", SceneLuaBindings::omodel_set_animation_time},
        {"animationSpeed", SceneLuaBindings::omodel_get_animation_speed},
        {"setAnimationSpeed", SceneLuaBindings::omodel_set_animation_speed},
        {"__tostring", SceneLuaBindings::omodel_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_remove(L, -1);
  }

  // material
  {
    luaL_newmetatable(L, "Material");
    lua_pushvalue(L, -1);           /* pushes the metatable */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
    luaL_Reg funcs[] = {
        {"color", SceneLuaBindings::material_get_color},
        {"setColor", SceneLuaBindings::material_set_color},
        {"texture", SceneLuaBindings::material_get_texture},
        {"setTexture", SceneLuaBindings::material_set_texture},
        {"__tostring", SceneLuaBindings::material_tostring},
        {"__gc", SceneLuaBindings::material_destructor},
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
        {"nthChildOf", SceneLuaBindings::pool_nth_child_of},
        {"root", SceneLuaBindings::pool_get_root},
        // {"remove", SceneLuaBindings::pool_remove},
        {"__tostring", SceneLuaBindings::pool_tostring},
        {NULL, NULL},
    };
    luaL_setfuncs(L, funcs, 0);
    lua_setmetatable(L, -2); // 1st - udata, 2nd - (meta)table
    lua_setfield(L, -2, "pool");
  }

  // lua_remove(L, -1);
  // lua_remove(L, -1);

  // for (int i = 1; i < 7; i++)
  //   std::cerr << "lua type: " << (int)(lua_type(L, i) - LUA_TNIL) << "\n";
}
