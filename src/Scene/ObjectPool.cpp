#include "Model.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Sheet.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <LuaBindings.hpp>
#include <ObjectPool.hpp>
#include <Scene.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <glad/gl.h>
#include <iomanip>
#include <iostream>
#include <sstream>

Text3D::~Text3D() {
  if (vao)
    glDeleteVertexArrays(1, &vao);

  if (vbo)
    glDeleteBuffers(1, &vbo);
}

Text3D::Text3D(const Text3D &other) { *this = other; }

Text3D &Text3D::operator=(const Text3D &other) {
  if (this != &other) {
    if (vao)
      glDeleteVertexArrays(1, &vao);

    if (vbo)
      glDeleteBuffers(1, &vbo);
    vbo = 0;
    vao = 0;
    // txt_len = other.txt_len;
    // length = other.length;
    // height = other.height;
    // v_capacity = other.v_capacity;
    text = other.text;
    // buf = other.buf;
    setText(text.c_str());
  }
  return *this;
};

Text3D::Text3D(Text3D &&other) noexcept { *this = std::move(other); };

Text3D &Text3D::operator=(Text3D &&other) noexcept {
  if (this != &other) {
    vbo = other.vbo;
    vao = other.vao;
    txt_len = other.txt_len;
    length = other.length;
    height = other.height;
    v_capacity = other.v_capacity;
    text = std::move(other.text);
    buf = std::move(other.buf);
    other.vbo = 0;
    other.vao = 0;
    other.txt_len = 0;
    other.length = 0;
    other.height = 0;
    other.v_capacity = 0;
  }
  return *this;
};

void Text3D::setText(const char *text_) {
  if (text_ != text.c_str())
    text = text_;
  if (text.size() > v_capacity) {
    if (!vbo)
      glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    buf.resize(text.capacity() * 6); // 6 verticies per character
    glBufferData(GL_ARRAY_BUFFER, sizeof(Text3D::Char) * text.capacity() * 6,
                 buf.data(), GL_DYNAMIC_DRAW);
    v_capacity = text.capacity();

    const Renderer::rLocation loc_charidx =
        glGetAttribLocation(Scene::text_program, "charIdx");

    const Renderer::rLocation loc_coloridx =
        glGetAttribLocation(Scene::text_program, "colorIdx");

    const Renderer::rLocation loc_linenr =
        glGetAttribLocation(Scene::text_program, "lineNr");

    const Renderer::rLocation loc_columnnr =
        glGetAttribLocation(Scene::text_program, "columnNr");

    if (!vao)
      glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(loc_charidx);
    glVertexAttribIPointer(loc_charidx, 1, GL_INT, sizeof(Text3D::Char),
                           (void *)offsetof(Text3D::Char, charIdx));
    glEnableVertexAttribArray(loc_coloridx);
    glVertexAttribIPointer(loc_coloridx, 1, GL_INT, sizeof(Text3D::Char),
                           (void *)offsetof(Text3D::Char, colorIdx));
    glEnableVertexAttribArray(loc_linenr);
    glVertexAttribIPointer(loc_linenr, 1, GL_INT, sizeof(Text3D::Char),
                           (void *)offsetof(Text3D::Char, lineNr));
    glEnableVertexAttribArray(loc_columnnr);
    glVertexAttribIPointer(loc_columnnr, 1, GL_INT, sizeof(Text3D::Char),
                           (void *)offsetof(Text3D::Char, columnNr));
    glBindVertexArray(0);
  }

  length = 0;
  height = 0;
  int column = 0;
  int color = 7;
  int charidx = 0;

  // memset(buf.data(), 0, sizeof(Text3D::Char) * buf.size());
  for (int i = 0; i < text.size(); i++) {
    char c = text[i];

    if (c == '\\' && i + 1 < text.size()) {
      if (text[i + 1] == '\\') {
        i++;
      } else {
        color =
            (text[i + 1] >= '0' && text[i + 1] <= '9')
                ? (int)((text[i + 1] - '0') % 8) // currently 8 colors (0 - 7)
                : 0;
        i++;
        continue;
      }

    } else if (c == '\n') {

      if (column > length)
        length = column;
      column = 0;
      height++;
      continue;
    }

    for (int j = 0; j < 6; j++)
      buf[(charidx * 6) + j] = Text3D::Char{
          .charIdx = text[i],
          .colorIdx = color,
          .lineNr = height,
          .columnNr = column,
      };
    charidx++;
    column++;
  }

  if (column > length)
    length = column;
  height++;
  txt_len = charidx;

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Text3D::Char) * text.size() * 6,
                  buf.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Text3D::draw(const glm::mat4 &pos) {
  glUseProgram(Scene::text_program);
  glUniform1i(Scene::m_text_length, length);
  glUniform1i(Scene::m_text_height, height);
  glUniformMatrix4fv(Scene::m_text_position, 1, false, glm::value_ptr(pos));
  glBindVertexArray(vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Scene::m_text_texture);
  glDrawArrays(GL_TRIANGLES, 0, txt_len * 6);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

void oDataPoint2::update_text(Sampler *displayed_row) {
  std::stringstream s;
  if (!displayed_row->ptr)
    return;

  int max_hdr_len = 0;
  for (auto &header : headers) {

    if (header.header_idx == -1 || sheet_idx != displayed_row->sheet_idx) {
      header.header_idx = -1;
      if (header.header.empty()) {
        continue;
      } else {
        auto it = displayed_row->headers.find(header.header);
        if (it != displayed_row->headers.end()) {
          header.header_idx = it->second;
        } else {
          continue;
        }
      }
    }

    if (header.header.size() > max_hdr_len)
      max_hdr_len = header.header.size();
  }
  sheet_idx = displayed_row->sheet_idx;

  int i = 0;
  for (auto &header : headers) {
    if (header.header_idx == -1)
      continue;

    s << "\\4|\\3" << header.header << ":\\7"
      << std::setw(max_hdr_len - header.header.size() + 1) << " ";

    const CellValue *cell = displayed_row->values.data() + header.header_idx;
    if (const NullCell *value = std::get_if<NullCell>(cell)) {
    } else if (const ErrorCell *value = std::get_if<ErrorCell>(cell)) {
      s << "[Błąd: " << value->msg << "]";
    } else if (const std::string *value = std::get_if<std::string>(cell)) {
      s << *value;
    } else if (const int64_t *value = std::get_if<int64_t>(cell)) {
      s << (*value >= header.min && *value <= header.max ? "\\2" : "\\1")
        << (int)*value;
    } else if (const double *value = std::get_if<double>(cell)) {
      s << (*value >= header.min && *value <= header.max ? "\\2" : "\\1")
        << std::fixed << std::setprecision(2) << *value;
    } else if (const bool *value = std::get_if<bool>(cell)) {
      s << (*value ? "true" : "false");
    } else if (const tm *value = std::get_if<tm>(cell)) {
      static char dtbuf[50] = {0};
      if (value->tm_mday == 0)
        strftime(dtbuf, sizeof(dtbuf), "%H:%M:%S", value);
      else
        strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", value);
      s << dtbuf;
    }
    if (i < headers.size())
      s << "\n";
    i++;
  }
  display_text.setText(s.str().c_str());
}

void Object::change_script_class(ObjectPool *pool, int new_script_class_idx) {
  if (script_class_idx != -1) {
    auto L = pool->get_lua();
    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_rawgeti(L, -1, script_class_idx + 1);
    lua_remove(L, -2);
    lua_getfield(L, -1, "on_detach");
    lua_remove(L, -2);
    Object **obj = (Object **)lua_newuserdata(L, sizeof(Object *));
    *obj = this;
    luaL_getmetatable(L, "ObjectRef");
    lua_setmetatable(L, -2);
    call_lua(L, 1);

    lua_params.clear();

    if (ref_to_self != -1) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_DATA_IDX);
      lua_pushnil(L);
      lua_rawseti(L, -2, ref_to_self + 1);
      lua_remove(L, -1);

      lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_IN_REG_IDX);
      Scene *scene = (*(Scene **)lua_touserdata(L, -1));
      scene->objPool.script_data_freelist.push_back(ref_to_self);
      ref_to_self = -1;
    }
    // lua_remove(L, -1);
  }

  script_class_idx = new_script_class_idx;

  if (script_class_idx != -1) {
    auto L = pool->get_lua();
    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_rawgeti(L, -1, script_class_idx + 1);
    lua_remove(L, -2);
    lua_getfield(L, -1, "on_attach");
    lua_remove(L, -2);
    Object **obj = (Object **)lua_newuserdata(L, sizeof(Object *));
    *obj = this;
    luaL_getmetatable(L, "ObjectRef");
    lua_setmetatable(L, -2);
    call_lua(L, 1);

    for (int i = 0;
         i < pool->get_script_classes()[script_class_idx].params.size(); i++) {
      lua_params.push_back(LuaParam{});
    }
  }
}

ObjectPool::ObjectPool() {
  objects.push_back(Object{.gen = 1,
                           .transform = glm::mat4(1.0f),
                           .name = "Scena",
                           .children = {},
                           .variant = oNode{}});
}

ObjectPool::ObjectPool(lua_State *L) {
  this->L = L;
  objects.push_back(Object{.gen = 1,
                           .transform = glm::mat4(1.0f),
                           .name = "Scena",
                           .children = {},
                           .variant = oNode{}});
}

Object *ObjectPool::get(objH obj) {
  if (obj.idx < 0 || obj.idx >= objects.size()) {
    // Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
    //          TL(MSG_OBJECT_POOL_GET_ERROR),
    //          std::make_format_args(obj.idx, obj.gen));
    return nullptr;
  }

  if (std::get_if<oEmptySlot>(&objects[obj.idx].variant))
    return nullptr;

  uint8_t gen = objects[obj.idx].gen;
  if (gen != obj.gen) {
    // Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
    //          TL(MSG_OBJECT_POOL_GET_ERROR),
    //          std::make_format_args(obj.idx, obj.gen));
    return nullptr;
  }

  return &objects[obj.idx];
}

objH ObjectPool::add(Object &&obj) {
  int16_t idx = 0;
  if (free_indices.size() != 0) {
    idx = free_indices[free_indices.size() - 1];
    free_indices.pop_back();
  } else {
    idx = objects.size();
    objects.push_back({});
  }

  uint8_t gen = objects[idx].gen;
  objects[idx] = std::move(obj);
  objects[idx].gen = ++gen;
  return objH{.gen = gen, .idx = idx};
}

void ObjectPool::remove(objH obj) {
  if (obj.idx <= 0 || obj.idx >= objects.size()) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_ERROR),
             std::make_format_args(obj.idx, obj.gen));
    return;
  }

  uint8_t gen = objects[obj.idx].gen;
  if (gen != obj.gen) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_ERROR),
             std::make_format_args(obj.idx, obj.gen));
    return;
  }

  if (std::get_if<oEmptySlot>(&objects[obj.idx].variant)) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Object pool",
             TL(MSG_OBJECT_POOL_REMOVE_EMPTY_SLOT_ERROR),
             std::make_format_args(obj.idx));
    return;
  }

  remove_impl(obj.idx);
}

void ObjectPool::remove_impl(int16_t idx) {
  for (int16_t child : objects[idx].children) {
    remove_impl(child);
  }
  Object &obj = objects[idx];
  obj.variant.emplace<oEmptySlot>(oEmptySlot{});
  if (obj.script_class_idx != -1) {
    obj.change_script_class(this, -1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, SCENE_IN_REG_IDX);
    Scene *scene = (*(*(Scene ***)lua_touserdata(L, -1)));
    scene->objPool.script_data_freelist.push_back(obj.ref_to_self);
  }
  free_indices.push_back(idx);
}

objH ObjectPool::add_model_node(ResourceManager &resMan, objH obj,
                                const std::string_view &model_name) {

  const char *name =
      model_name.empty()
          ? (resMan.GetModelsMap().size() == 0
                 ? ""
                 : (*resMan.GetModelsMap().begin()).first.c_str())
          : model_name.data();

  ManagedModel *model = resMan.GetModel(name);
  if (model == nullptr)
    return objH{.gen = 0, .idx = -1};

  // auto model_name_ = std::make_shared<Eval::Model>();
  // model_name_.get()->name = name;

  // auto materials = std::make_shared<std::vector<Eval::Value>>();
  // for (const auto &material : model->materials) {
  //   auto mat = std::make_shared<Eval::Material>();
  //   *mat.get() = Eval::Material{.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
  //                               .diffuse1 = material.diffuse1};
  //
  //   materials.get()->push_back(Eval::Value{.data = mat});
  // }

  oModel omodel{};
  for (const auto &material : model->materials) {
    omodel.materials.push_back(Material{.color_diffuse = glm::vec4(1.0f),
                                        .diffuse1 = material.diffuse1});
  }

  omodel.model = name;
  // omodel.materials = materials;
  omodel.animation_idx = -1;
  omodel.animation_time = 0;
  omodel.animation_speed = 1.0f;

  objH child =
      add(Object{.parent = obj, .name = name, .variant = std::move(omodel)});
  get(obj)->children.push_back(child.idx);
  return child;
}

objH ObjectPool::add_node(objH obj) {
  objH child = add(Object{.parent = obj, .name = "dane", .variant = oNode{}});
  get(obj)->children.push_back(child.idx);
  return child;
}

objH ObjectPool::add_datapoint_node(objH obj) {
  objH child =
      add(Object{.parent = obj, .name = "dane", .variant = oDataPoint2{}});
  get(obj)->children.push_back(child.idx);
  return child;
}

objH ObjectPool::add_text_node(objH obj) {
  objH child = add(Object{.parent = obj, .name = "napis", .variant = Text3D{}});
  get(obj)->children.push_back(child.idx);
  return child;
}
