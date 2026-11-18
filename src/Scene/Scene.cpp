#include "LuaBindings.hpp"
#include "Model.hpp"
#include "ObjectPool.hpp"
#include "ResourceManager.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "images.h"
#include <cstring>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/ext/matrix_clip_space.hpp>
#include "glm/common.hpp"
#include "glm/trigonometric.hpp"
#include <glm/ext/matrix_transform.hpp>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include <yyjson.h>

#include <FileUtils.hpp>
#include <Logging.hpp>
#include <Renderer.hpp>
#include <Renderer_internal.hpp>
#include <Scene.hpp>
#include <Sheet.hpp>
#include <cstdio>
#include <iostream>

#include <shaders.h>
#include <staticassets_scene.h>
#include <stdatomic.h>

#include <ImGuizmo.h>
static ImGuizmo::OPERATION mCurrentGizmoOperation(
    (ImGuizmo::OPERATION)((int)ImGuizmo::UNIVERSAL | (int)ImGuizmo::BOUNDS));
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

Renderer::rProgram Scene::text_program = 0;
Renderer::rProgram Scene::m_text_texture = 0;
Renderer::rProgram Scene::m_text_length = 0;
Renderer::rProgram Scene::m_text_height = 0;
Renderer::rProgram Scene::m_text_position = 0;
Renderer::rProgram Scene::m_text_ID = 0;
Renderer::rProgram Scene::m_text_view = 0;
Renderer::rProgram Scene::m_text_projection = 0;
Renderer::rProgram Scene::m_text_pixel_size = 0;
Renderer::rProgram Scene::skybox_program = 0;
Renderer::rFBO Scene::skybox_fbo = 0;
Renderer::rFBO Scene::skybox_ebo = 0;
Renderer::rVAO Scene::skybox_vao = 0;
Renderer::rProgram Scene::selection_bb_program = 0;
Renderer::rFBO Scene::selection_bb_fbo = 0;
Renderer::rVAO Scene::selection_bb_vao = 0;
Renderer::rProgram Scene::tri_program = 0;
Renderer::rFBO Scene::tri_fbo = 0;
Renderer::rVAO Scene::tri_vao = 0;

Renderer::rProgram Scene::skinning_program = 0;

Renderer::rLocation Scene::skybox_loc = 0;
Renderer::rLocation Scene::skybox_view_loc = 0;
Renderer::rLocation Scene::skybox_projection_loc = 0;

Renderer::rLocation Scene::model_view_loc = 0;
Renderer::rLocation Scene::model_projection_loc = 0;
Renderer::rLocation Scene::model_ID_loc = 0;
Renderer::rLocation Scene::model_model_loc = 0;

Renderer::rLocation Scene::selection_bb_view_loc = 0;
Renderer::rLocation Scene::selection_bb_projection_loc = 0;
Renderer::rLocation Scene::selection_bb_model_loc = 0;
Renderer::rLocation Scene::selection_bb_min_loc = 0;
Renderer::rLocation Scene::selection_bb_max_loc = 0;

static GLuint skybox_indiecies[] = {
    0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

struct skybox_vert {
  Renderer::vec3 pos;
  Renderer::vec2 uv;
};

static skybox_vert skybox_verticies[] = {
    // back
    {{-1.0, 1.0, -1.0}, {0.75, 0.665}},
    {{-1.0, -1.0, -1.0}, {0.75, 0.334}},
    {{1.0, -1.0, -1.0}, {1.00, 0.334}},
    {{1.0, 1.0, -1.0}, {1.00, 0.665}},

    // front
    {{1.0, 1.0, 1.0}, {0.25, 0.665}},
    {{1.0, -1.0, 1.0}, {0.25, 0.334}},
    {{-1.0, -1.0, 1.0}, {0.50, 0.334}},
    {{-1.0, 1.0, 1.0}, {0.50, 0.665}},

    // right
    {{1.0, 1.0, -1.0}, {0.00, 0.665}},
    {{1.0, -1.0, -1.0}, {0.00, 0.334}},
    {{1.0, -1.0, 1.0}, {0.25, 0.334}},
    {{1.0, 1.0, 1.0}, {0.25, 0.665}},

    // left
    {{-1.0, 1.0, 1.0}, {0.50, 0.665}},
    {{-1.0, -1.0, 1.0}, {0.50, 0.334}},
    {{-1.0, -1.0, -1.0}, {0.75, 0.334}},
    {{-1.0, 1.0, -1.0}, {0.75, 0.665}},

    // bottom
    {{-1.0, -1.0, -1.0}, {0.499, 0.000}},
    {{-1.0, -1.0, 1.0}, {0.499, 0.332}},
    {{1.0, -1.0, 1.0}, {0.251, 0.332}},
    {{1.0, -1.0, -1.0}, {0.251, 0.000}},

    // top
    {{-1.0, 1.0, 1.0}, {0.499, 0.667}},
    {{-1.0, 1.0, -1.0}, {0.499, 1.000}},
    {{1.0, 1.0, -1.0}, {0.251, 1.000}},
    {{1.0, 1.0, 1.0}, {0.251, 0.667}},
};

static float selection_cube_vertices[] = {
    -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,  0.5,  0.5,  0.5,
    -0.5, -0.5, 0.5,  0.5,  0.5,  0.5,  -0.5, 0.5,  0.5,

    -0.5, -0.5, -0.5, -0.5, 0.5,  -0.5, 0.5,  0.5,  -0.5,
    -0.5, -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,  -0.5, -0.5,

    -0.5, 0.5,  -0.5, -0.5, 0.5,  0.5,  0.5,  0.5,  0.5,
    -0.5, 0.5,  -0.5, 0.5,  0.5,  0.5,  0.5,  0.5,  -0.5,

    -0.5, -0.5, -0.5, 0.5,  -0.5, -0.5, 0.5,  -0.5, 0.5,
    -0.5, -0.5, -0.5, 0.5,  -0.5, 0.5,  -0.5, -0.5, 0.5,

    0.5,  -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,  0.5,  0.5,
    0.5,  -0.5, -0.5, 0.5,  0.5,  0.5,  0.5,  -0.5, 0.5,

    -0.5, -0.5, -0.5, -0.5, -0.5, 0.5,  -0.5, 0.5,  0.5,
    -0.5, -0.5, -0.5, -0.5, 0.5,  0.5,  -0.5, 0.5,  -0.5};

static float text_bpos[] = {
    -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
};

static float text_tpos[] = {
    0.0, 0.0, 0.0,    -0.0625, 0.0625, -0.0625,
    0.0, 0.0, 0.0625, -0.0625, 0.0625, 0.0,
};

consteval glm::vec4 rgba2vec4(uint32_t in) {
  return glm::vec4((in / (256 * 256 * 256)) / 255.0,
                   ((in / (256 * 256)) % 256) / 255.0,
                   ((in / 256) % 256) / 256.0, (in % 256) / 255.0);
}

static glm::vec4 txt_colors[] = {
    rgba2vec4(0x111111FF), rgba2vec4(0xDE5D68FF), rgba2vec4(0x8FD573FF),
    rgba2vec4(0xDBB671FF), rgba2vec4(0x57A5E5FF), rgba2vec4(0xBB70D2FF),
    rgba2vec4(0x51A8B3FF), rgba2vec4(0xCCCCCCFF),
};

struct Vertex {
  Renderer::vec2 pos;
  Renderer::vec3 col;
};

static const Vertex vertices[3] = {{{0.0f, 0.5f}, {1.f, 0.f, 0.f}},
                                   {{-0.5f, -0.5f}, {0.f, 1.f, 0.f}},
                                   {{0.5f, -0.5f}, {0.f, 0.f, 1.f}}};

static bool
write_file_if_not_exists_reported(const std::filesystem::path &filepath,
                                  const void *data, size_t size) {

  auto err = FileUtils::WriteFileIfNotExists(filepath, data, size);
  if (!err.has_value()) {
    std::string path = filepath.string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             err.error() == -1 ? TL(MSG_GENERIC_OPEN_ERROR)
                               : TL(MSG_GENERIC_WRITE_ERROR),
             std::make_format_args(path));
    return false;
  }
  return true;
}

bool Scene::create_folder_structure(const std::string &name,
                                    const std::filesystem::path &path) {
  try {
    std::filesystem::create_directory(path);
    std::filesystem::create_directory(path / "models");
    std::filesystem::create_directory(path / "textures");
    std::filesystem::create_directory(path / "excel_sheets");
    std::filesystem::create_directory(path / "scripts");
  } catch (std::filesystem::filesystem_error &err) {
    std::string path1 = err.path1().string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             TL(MSG_SCENE_DIR_CREATE_ERROR), std::make_format_args(path1));
    return false;
  }

  if (!write_file_if_not_exists_reported(path / "scene.json", scene_json_data,
                                         scene_json_size))
    return false;

  if (!write_file_if_not_exists_reported(path / "skybox.png", skybox_png_data,
                                         skybox_png_size))
    return false;

  if (!write_file_if_not_exists_reported(path / "excel_sheets/data.xlsx",
                                         data_xlsx_data, data_xlsx_size))
    return false;

  return true;
}

Scene::Scene(const std::string &name) {
  this->name = name;
  current_folder = FileUtils::APP_ROOT / "scenes" / std::filesystem::path(name);
  if (!create_folder_structure(name, current_folder)) {
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene", TL(MSG_SCENE_CREATE_ERROR),
             std::make_format_args(name));
    throw std::runtime_error("");
  }

  Log::log(Log::DEFAULT, 0, "Scene", TL(MSG_SCENE_OPEN_SUCCESS),
           std::make_format_args(name));

  resMan = std::move(ResourceManager{current_folder});

  init_lua(&L);
  add_lua(L);
  objPool = ObjectPool(L);

  for (auto &ent :
       std::filesystem::directory_iterator(current_folder / "scripts")) {
    if (std::filesystem::is_directory(ent.path())) {
      auto script_path = ent.path() / "init.lua";
      if (FileUtils::FileExists(script_path)) {
        auto res_fs = FileUtils::ReadFile(script_path);
        if (res_fs.has_value()) {
          std::string path = script_path.string();
          auto rr = res_fs.value();
          if (LUA_OK != lua_load(
                            L,
                            [](lua_State *L, void *data, size_t *size) {
                              auto *rr = (FileUtils::ReadResult *)data;
                              if (rr->length <= 0)
                                return (const char *)nullptr;
                              *size = rr->length;
                              rr->length -= *size;
                              return (const char *)rr->data;
                            },
                            &rr, path.c_str(), "bt")) {
            std::cerr << "\x1b[31m" << lua_tostring(L, -1) << "\x1b[0m\n";
            lua_pop(L, 1);
          }
          call_lua(L);
          delete[] res_fs.value().data;
        } else {
          std::string path = script_path.string();
          switch (res_fs.error()) {
          case -2:
            Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
                     TL(MSG_GENERIC_FILE_NOT_FOUND),
                     std::make_format_args(path));
            break;

          case -1:
            Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
                     TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));
            break;

          case 1:
            Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
                     TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));
            break;

          default:
            break;
          }
          throw std::runtime_error("");
        }
      }
    }
  }

  int lftype = lua_getglobal(L, "HEADERS");
  if (lftype != LUA_TTABLE && lftype != LUA_TNIL) {
    lua_remove(L, -1);
    throw std::runtime_error("zmienna HEADERS nie jest listą");
  }

  if (lftype == LUA_TTABLE) {
    int len = luaL_len(L, -1);
    for (int i = 0; i < len; i++) {
      if (lua_rawgeti(L, -1, i + 1) != LUA_TSTRING) {
        lua_remove(L, -1);
        lua_remove(L, -1);
        throw std::runtime_error(
            "element w liście HEADERS nie jest ciągiem znaków");
      }

      const char *hdr = lua_tostring(L, -1);
      sampler.sampler2sheet.push_back(-1);
      sampler.header_names.push_back(hdr);
      sampler.headers[hdr] = i;
      sampler.values.push_back(NullCell{});
      lua_remove(L, -1);
    }
  }
  lua_remove(L, -1);

  for (auto &ent :
       std::filesystem::directory_iterator(current_folder / "excel_sheets")) {
    if (std::filesystem::is_regular_file(ent.path())) {
      std::cerr << "loading " << ent.path() << "\n";
      auto sheet_res = ReadXLSXfile(ent.path(), ++next_sheet_idx);
      if (sheet_res.has_value()) {
        auto &sheet = sheet_res.value();
        for (int i = 0; i < sheet.headers.size(); i++) {
          auto it = sampler.headers.find(sheet.headers[i]);
          if (it != sampler.headers.end()) {
            sheet.sheet2sampler[i] = it->second;
          }
        }
        sheets[ent.path().stem().string()] = sheet;
      }
    }
  }

  if (!load())
    throw std::runtime_error("");
}

bool Scene::create_framebuffer() {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

  glGenTextures(1, &screen_canvas);
  glBindTexture(GL_TEXTURE_2D, screen_canvas);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screen_canvas, 0);

  glGenTextures(1, &color_canvas);
  glBindTexture(GL_TEXTURE_2D, color_canvas);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         color_canvas, 0);

  glGenRenderbuffers(1, &renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 480);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer);

  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(3, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Log::log_uform(Log::ERROR | Log::SEV_MED, 0, "Scene",
                   TL(MSG_SCENE_FRAMEBUFFER_CREATE_ERROR));
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  Log::log(Log::DEBUG, 0, "Scene", TL(MSG_SCENE_FRAMEBUFFER_CREATE_SUCCESS),
           std::make_format_args(screen_canvas, color_canvas));

  return true;
}

bool Scene::resize(Renderer::rect_size size) {
  if (this->size.width == size.width && this->size.height == size.height)
    return false;

  this->size = size;
  float pixel_size[2] = {1.0f / size.width, 1.0f / size.height};
  glUseProgram(text_program);
  glUniform2fv(m_text_pixel_size, 1, pixel_size);
  glUseProgram(0);
  resize_framebuffer(size);

  projection =
      glm::perspective(glm::radians(90.0f),
                       (float)size.width / (float)size.height, 0.01f, 100.0f);

  return true;
}

void Scene::resize_framebuffer(Renderer::rect_size size) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glBindTexture(GL_TEXTURE_2D, screen_canvas);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screen_canvas, 0);

  glBindTexture(GL_TEXTURE_2D, color_canvas);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         color_canvas, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.width,
                        size.height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Log::log(Log::VERBOSE, 0, "Scene", TL(MSG_SCENE_FRAMEBUFFER_RESIZE),
           std::make_format_args(screen_canvas, color_canvas, size.width,
                                 size.height));
}

bool Scene::Init(Renderer::Render &render) {
  auto res_shader = render.LoadProgram("simple", (const char *)simple_vs_data,
                                       (const char *)simple_fs_data);

  if (!res_shader.has_value())
    return false;

  tri_program = res_shader.value();

  res_shader = render.LoadProgram("skybox", (const char *)skybox_vs_data,
                                  (const char *)skybox_fs_data);

  if (!res_shader.has_value()) {
    render.UnloadProgram(tri_program);
    tri_program = 0;
    return false;
  }

  skybox_program = res_shader.value();

  res_shader = render.LoadProgram("skinning", (const char *)skinning_vs_data,
                                  (const char *)skinning_fs_data);

  if (!res_shader.has_value()) {
    render.UnloadProgram(tri_program);
    render.UnloadProgram(skybox_program);
    tri_program = 0;
    skybox_program = 0;
    return false;
  }

  skinning_program = res_shader.value();

  res_shader = render.LoadProgram("selection_bounding_box",
                                  (const char *)selection_bb_vs_data,
                                  (const char *)selection_bb_fs_data);

  if (!res_shader.has_value()) {
    render.UnloadProgram(tri_program);
    render.UnloadProgram(skybox_program);
    render.UnloadProgram(skinning_program);
    tri_program = 0;
    skybox_program = 0;
    skinning_program = 0;
    return false;
  }

  selection_bb_program = res_shader.value();

  res_shader = render.LoadProgram("text_datapoint", (const char *)text_vs_data,
                                  (const char *)text_fs_data);

  if (!res_shader.has_value()) {
    render.UnloadProgram(tri_program);
    render.UnloadProgram(skybox_program);
    render.UnloadProgram(skinning_program);
    render.UnloadProgram(selection_bb_program);
    tri_program = 0;
    skybox_program = 0;
    skinning_program = 0;
    selection_bb_program = 0;
    return false;
  }

  text_program = res_shader.value();
  glUseProgram(text_program);
  glUniform2fv(glGetUniformLocation(text_program, "bpos"), 6, text_bpos);
  glUniform2fv(glGetUniformLocation(text_program, "tpos"), 6, text_tpos);
  glUniform4fv(glGetUniformLocation(text_program, "colors"),
               sizeof(txt_colors) / sizeof(glm::vec4), (float *)txt_colors);
  m_text_pixel_size = glGetUniformLocation(text_program, "psize");
  m_text_view = glGetUniformLocation(text_program, "view");
  m_text_ID = glGetUniformLocation(text_program, "ID");
  m_text_projection = glGetUniformLocation(text_program, "projection");
  m_text_position = glGetUniformLocation(text_program, "pos");
  m_text_length = glGetUniformLocation(text_program, "length");
  m_text_height = glGetUniformLocation(text_program, "height");

  auto res_img = Renderer::Render::sLoadTexture(
      Renderer::Image{.width = font_png_width,
                      .height = font_png_height,
                      .channels = font_png_channels,
                      .mipmap_levels = 2,
                      .pixels = (unsigned char *)font_png_pixels},
      true);

  if (!res_img.has_value()) {
    render.UnloadProgram(tri_program);
    render.UnloadProgram(skybox_program);
    render.UnloadProgram(skinning_program);
    render.UnloadProgram(selection_bb_program);
    render.UnloadProgram(text_program);
    tri_program = 0;
    skybox_program = 0;
    skinning_program = 0;
    selection_bb_program = 0;
    text_program = 0;
    glUseProgram(0);
    return false;
  }
  m_text_texture = res_img.value();
  glUniform1i(glGetUniformLocation(text_program, "text"), 0);
  glUseProgram(0);

  model_view_loc = glGetUniformLocation(skinning_program, "view");
  model_projection_loc = glGetUniformLocation(skinning_program, "projection");
  model_model_loc = glGetUniformLocation(skinning_program, "model");
  model_ID_loc = glGetUniformLocation(skinning_program, "ID");

  Renderer::Model::setDefaultProgram(skinning_program);

  selection_bb_view_loc = glGetUniformLocation(selection_bb_program, "view");
  selection_bb_projection_loc =
      glGetUniformLocation(selection_bb_program, "projection");
  selection_bb_model_loc = glGetUniformLocation(selection_bb_program, "model");
  selection_bb_min_loc = glGetUniformLocation(selection_bb_program, "min");
  selection_bb_max_loc = glGetUniformLocation(selection_bb_program, "max");

  glGenBuffers(1, &selection_bb_fbo);
  glBindBuffer(GL_ARRAY_BUFFER, selection_bb_fbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(selection_cube_vertices),
               selection_cube_vertices, GL_STATIC_DRAW);

  const Renderer::rLocation bb_position_loc =
      glGetAttribLocation(selection_bb_program, "position");

  glGenVertexArrays(1, &selection_bb_vao);
  glBindVertexArray(selection_bb_vao);
  glEnableVertexAttribArray(bb_position_loc);
  glVertexAttribPointer(bb_position_loc, 3, GL_FLOAT, GL_FALSE,
                        sizeof(float) * 3, (void *)0);
  glBindVertexArray(0);

  glGenBuffers(1, &tri_fbo);
  glBindBuffer(GL_ARRAY_BUFFER, tri_fbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const Renderer::rLocation position_loc =
      glGetAttribLocation(tri_program, "position");
  const Renderer::rLocation color_loc =
      glGetAttribLocation(tri_program, "color");

  glGenVertexArrays(1, &tri_vao);
  glBindVertexArray(tri_vao);
  glEnableVertexAttribArray(position_loc);
  glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, pos));
  glEnableVertexAttribArray(color_loc);
  glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, col));
  glBindVertexArray(0);

  skybox_projection_loc = glGetUniformLocation(skybox_program, "projection");
  skybox_view_loc = glGetUniformLocation(skybox_program, "view");

  glGenBuffers(1, &skybox_fbo);
  glBindBuffer(GL_ARRAY_BUFFER, skybox_fbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_verticies), skybox_verticies,
               GL_STATIC_DRAW);

  glGenBuffers(1, &skybox_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skybox_indiecies),
               skybox_indiecies, GL_STATIC_DRAW);

  const Renderer::rLocation pos_loc =
      glGetAttribLocation(skybox_program, "pos");
  const Renderer::rLocation uv_loc = glGetAttribLocation(skybox_program, "uv");

  glGenVertexArrays(1, &skybox_vao);
  glBindVertexArray(skybox_vao);
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(skybox_vert),
                        (void *)offsetof(skybox_vert, pos));
  glEnableVertexAttribArray(uv_loc);
  glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(skybox_vert),
                        (void *)offsetof(skybox_vert, uv));

  return true;
}

bool Scene::init(Renderer::Render &render) {
  if (!create_framebuffer())
    return false;

  std::filesystem::path filepath = current_folder / "skybox.png";
  auto res_img_read = render.LoadImage(FileUtils::RealFs{}, filepath);

  if (!res_img_read.has_value()) {
    std::string path = filepath.string();
    switch (res_img_read.error()) {
    case -2:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_FILE_NOT_FOUND), std::make_format_args(path));
      break;

    case -1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));
      break;

    case 1:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));
      break;

    case 2:
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));
      break;
    default:
      break;
    }

    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialised = false;
    return false;
  }

  auto res_txt = render.LoadTexture(res_img_read.value());

  if (!res_txt.has_value()) {
    std::string path = filepath.string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));

    free(res_img_read.value().pixels);

    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialised = false;
    return false;
  }

  free(res_img_read.value().pixels);

  skybox_texture = res_txt.value();
  skybox_loc = glGetUniformLocation(skybox_program, "skybox");
  glUniform1i(skybox_loc, 0);

  for (auto &ent :
       std::filesystem::directory_iterator(current_folder / "models")) {
    if (std::filesystem::is_directory(ent.path())) {
      FileUtils::path path = ent.path() / ent.path().filename();
      auto res_model =
          Renderer::Model::LoadFromFile(FileUtils::RealFs{}, path, false);
      if (!res_model.has_value()) {
        if (!res_img_read.has_value()) {
          std::string path = path;
          Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
                   TL(MSG_GENERIC_FILE_NOT_FOUND), std::make_format_args(path));

          free(res_img_read.value().pixels);

          glDeleteFramebuffers(1, &framebuffer);
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          initialised = false;
          return false;
        }
      }
      auto e1 =
          resMan.ImportModel(FileUtils::RealFs{}, path, &res_model.value());
    }
  }

  initialised = true;
  atomic_store(&is_busy, false);

  // // ---------------------- model test
  // auto e1 = resMan.ImportModel(FileUtils::RealFs{},
  //                              std::filesystem::path{"assets"} / "example"
  //                              /
  //                                  "models" / "CesiumMan.m3d");
  // resMan.GetModel("CesiumMan.m3d")
  //     ->model.SetAnimation(
  //         &resMan.GetModel("CesiumMan.m3d")->model.GetAnimations()[0]);
  return true;
}

void Scene::cleanup() {
  close_lua(L);
  save();

  if (initialised) {
    glDeleteTextures(1, &skybox_texture);
    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initialised = false;
  }
}

void Scene::updateCamera() {
  // float sinx = glm::sin(body.orientation[0]);
  // float cosx = glm::sin(body.orientation[0]);
  // float siny = glm::sin(body.orientation[1]);
  float cosy = glm::cos(body.orientation.y);

  glm::vec3 forward(glm::sin(body.orientation.x) * cosy,
                    glm::sin(body.orientation.y),
                    glm::cos(body.orientation.x) * cosy);

  glm::vec3 right = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::vec3 up = glm::cross(right, forward);

  view = glm::lookAt(body.position, body.position + forward, up);
}

static glm::vec3 vec3clampZ(glm::vec3 vec, float minRange, float max) {
  float length = vec.length();
  return length <= max
             ? (length >= minRange ? vec : glm::vec3(0.0f, 0.0f, 0.0f))
             : (vec / length) * max;
}

void Scene::updateCameraAndBody(float delta) {
  glm::vec2 temp = glm::vec2((float)-mpos.width, (float)mpos.height) * 0.2f;
  // temp.x /= size.width;
  // temp.y /= size.height;
  if (mousebuttonR) {
    body.orientation += glm::radians(temp - last_mpos);
    body.orientation.x = glm::mod(body.orientation.x, glm::radians(360.0f));
    body.orientation.y =
        glm::clamp(body.orientation.y, glm::radians(-90.0f) + 0.001f,
                   glm::radians(90.0f) - 0.001f);
  }
  last_mpos = temp;

  float sinx = glm::sin(body.orientation.x);
  float cosx = glm::cos(body.orientation.x);
  float cosy = glm::cos(body.orientation.y);

  glm::vec3 forward(sinx * cosy, glm::sin(body.orientation.y), cosx * cosy);
  glm::vec3 right = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::vec3 up = glm::cross(right, forward);

  glm::vec3 movement_right(-cosx, 0.0, sinx);
  glm::vec3 movement_up = glm::cross(movement_right, forward);

  glm::vec3 move = glm::vec3(forward * input.x + movement_right * input.y +
                             movement_up * input.z);

  body.acceleration =
      vec3clampZ(body.velocity * -2.0f, 0.001, 5) + (20.0f * move);
  body.position +=
      delta *
      vec3clampZ(body.velocity + (body.acceleration * delta * 0.5f), 0.01, 5);
  body.velocity =
      vec3clampZ(body.velocity + body.acceleration * delta, 0.01, 5);
  view = glm::lookAt(body.position, body.position + forward, up);
}

void Scene::render(Renderer::Render &render) {
  static char uniformNameBuffer[100];
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

  glViewport(0, 0, size.width, size.height);
  glClearColor(0, 0, 0, 0);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(skybox_program);
  updateCameraAndBody(render.GetDelta());

  glUniformMatrix4fv(skybox_view_loc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(skybox_projection_loc, 1, GL_FALSE,
                     glm::value_ptr(projection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, skybox_texture);
  glBindVertexArray(skybox_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_ebo);
  glDrawElements(GL_TRIANGLES, sizeof(skybox_indiecies) / sizeof(GLuint),
                 GL_UNSIGNED_INT, 0);
  glClear(GL_DEPTH_BUFFER_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glUseProgram(skinning_program);

  glUniformMatrix4fv(model_view_loc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(model_projection_loc, 1, GL_FALSE,
                     glm::value_ptr(projection));

  glUseProgram(text_program);

  glUniformMatrix4fv(m_text_view, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(m_text_projection, 1, GL_FALSE,
                     glm::value_ptr(projection));

  // glm::mat4 model = glm::mat4(1.0f);
  // glUniformMatrix4fv(model_model_loc, 1, GL_FALSE, glm::value_ptr(model));
  // resMan.GetModel("CesiumMan.m3d")->model.Advance(render.GetDelta());
  // resMan.GetModel("CesiumMan.m3d")->model.Draw();

  render_selection_bb = false;
  draw_models_recursive(objPool.rootH().idx, render.GetDelta(), glm::mat4(1.0f),
                        objPool.rootH());

  glClear(GL_DEPTH_BUFFER_BIT);

  draw_onscreen_recursive(objPool.rootH().idx, render.GetDelta(),
                          glm::mat4(1.0f), objPool.rootH());

  glClear(GL_DEPTH_BUFFER_BIT);

  sampler.emit_update = false;

  if (mousebuttonLclick && mpos.width <= size.width &&
      mpos.height <= size.height) {

    unsigned char pixel[4] = {0};
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(mpos.width, mpos.height, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 &pixel);

    // if (pixel[3] == 255) {
    objH obj =
        objH{.gen = pixel[0], .idx = (int16_t)(pixel[1] + (pixel[2] * 256))};
    if (selected_object.idx == 0 ||
        (!ImGuizmo::IsOver() && selected_object.idx != 0))
      selected_object = objPool.get(obj) ? obj : objPool.rootH();
    // }

    // std::cerr << "objH pixel is " << (int)pixel[0] << ", "
    //           << (int)(pixel[1] + (pixel[2] * 256)) << " alpha "
    //           << (int)pixel[3] << "\n";
  }

  if (render_selection_bb) {
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(selection_bb_program);
    glBindVertexArray(selection_bb_vao);

    glUniformMatrix4fv(selection_bb_view_loc, 1, GL_FALSE,
                       glm::value_ptr(view));
    glUniformMatrix4fv(selection_bb_projection_loc, 1, GL_FALSE,
                       glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0,
                 sizeof(selection_cube_vertices) / sizeof(float));
  }

  // glUseProgram(tri_program);
  // glBindVertexArray(tri_vao);
  // glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool Scene::save() {
  yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
  yyjson_mut_val *root = yyjson_mut_obj(doc);
  yyjson_mut_doc_set_root(doc, root);

  yyjson_mut_obj_add_int(doc, root, "format-version", 0);
  yyjson_mut_obj_add_int(doc, root, "app-version", 0);
  yyjson_mut_obj_add_str(doc, root, "name", name.c_str());

  char datebuf[50] = {0};
  time_t t = time(NULL);
  tm *time_ = localtime(&t);
  strftime(datebuf, 50, "%Y-%m-%d %H-%M-%S", time_);

  yyjson_mut_obj_add_str(doc, root, "last-modified", datebuf);
  yyjson_mut_obj_add_str(doc, root, "selected-sheet", selected_sheet.c_str());

  yyjson_mut_val *sheet2sampler =
      yyjson_mut_obj_add_obj(doc, root, "sheet2sampler");
  for (const auto &[name, sheet] : sheets) {
    yyjson_mut_val *sheet_json =
        yyjson_mut_obj_add_obj(doc, sheet2sampler, name.c_str());
    for (const auto &[sheetidx, sampleridx] : sheet.sheet2sampler) {
      yyjson_mut_obj_add_int(doc, sheet_json, sheet.headers[sheetidx].c_str(),
                             sampleridx);
    }
  }

  int16_t counter;
  std::unordered_map<int16_t, int16_t> pool2json;
  yyjson_mut_val *objects = yyjson_mut_obj_add_arr(doc, root, "objects");

  save_object_pool_recursive(doc, objects, &counter, &pool2json, 0);

  char *json_string =
      yyjson_mut_write(doc, YYJSON_WRITE_PRETTY_TWO_SPACES, NULL);
  yyjson_mut_doc_free(doc);
  // std::cerr << json_string << "\n";

  auto res_fs = FileUtils::WriteFile(current_folder / "scene.json", json_string,
                                     strlen(json_string));
  if (!res_fs.has_value()) {
    std::string path = (current_folder / "scene.json").string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             res_fs.error() == -1 ? TL(MSG_GENERIC_OPEN_ERROR)
                                  : TL(MSG_GENERIC_WRITE_ERROR),
             std::make_format_args(path));
    return false;
  }

  return true;
}

static int16_t AddToPool2Json(std::unordered_map<int16_t, int16_t> *pool2json,
                              int16_t idx, int16_t *counter) {

  auto it = pool2json->find(idx);
  if (it == pool2json->end()) {
    (*pool2json)[idx] = (*counter)++;
    return (*counter) - 1;
  } else {
    return it->second;
  }
}

int16_t Scene::save_object_pool_recursive(
    void *doc_, void *pool_, int16_t *counter,
    std::unordered_map<int16_t, int16_t> *pool2json, int16_t idx) {

  auto doc = (yyjson_mut_doc *)doc_;
  auto pool = (yyjson_mut_val *)pool_;

  Object *node = &objPool.get(objPool.rootH())[idx];
  int16_t jdx = AddToPool2Json(pool2json, idx, counter);

  yyjson_mut_val *node_json = yyjson_mut_arr_add_obj(doc, pool);
  yyjson_mut_obj_add_str(doc, node_json, "name", node->name.c_str());
  yyjson_mut_obj_add_bool(doc, node_json, "collapsed", node->collapsed);
  if (node->script_class_idx != -1)
    yyjson_mut_obj_add_int(doc, node_json, "script-class",
                           node->script_class_idx);

  if (idx) {
    float translation[3];
    float rotation[3];
    float scale[3];

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(node->transform),
                                          translation, rotation, scale);

    yyjson_mut_val *translation_json =
        yyjson_mut_obj_add_arr(doc, node_json, "translation");
    yyjson_mut_val *rotation_json =
        yyjson_mut_obj_add_arr(doc, node_json, "rotation");
    yyjson_mut_val *scale_json =
        yyjson_mut_obj_add_arr(doc, node_json, "scale");

    yyjson_mut_arr_add_float(doc, translation_json, translation[0]);
    yyjson_mut_arr_add_float(doc, translation_json, translation[1]);
    yyjson_mut_arr_add_float(doc, translation_json, translation[2]);

    yyjson_mut_arr_add_float(doc, rotation_json, rotation[0]);
    yyjson_mut_arr_add_float(doc, rotation_json, rotation[1]);
    yyjson_mut_arr_add_float(doc, rotation_json, rotation[2]);

    yyjson_mut_arr_add_float(doc, scale_json, scale[0]);
    yyjson_mut_arr_add_float(doc, scale_json, scale[1]);
    yyjson_mut_arr_add_float(doc, scale_json, scale[2]);
  }

  if (const oEmptySlot *variant = std::get_if<oEmptySlot>(&node->variant)) {
    yyjson_mut_obj_add_str(doc, node_json, "variant", "EMPTY");

  } else if (const oNode *variant = std::get_if<oNode>(&node->variant)) {
    yyjson_mut_obj_add_str(doc, node_json, "variant", "NODE");

  } else if (const oModel *variant = std::get_if<oModel>(&node->variant)) {
    yyjson_mut_val *v = yyjson_mut_obj_add_arr(doc, node_json, "variant");
    yyjson_mut_arr_add_str(doc, v, "MODEL");
    yyjson_mut_arr_add_str(doc, v, variant->model.c_str());
    yyjson_mut_arr_add_int(doc, v, variant->animation_idx);
    yyjson_mut_arr_add_float(doc, v, variant->animation_time);
    yyjson_mut_arr_add_float(doc, v, variant->animation_speed);

    yyjson_mut_val *materials = yyjson_mut_arr_add_arr(doc, v);
    for (const auto &material : variant->materials) {
      yyjson_mut_val *m = yyjson_mut_arr_add_arr(doc, materials);
      yyjson_mut_arr_add_str(doc, m, material.diffuse1.c_str());
      yyjson_mut_val *c = yyjson_mut_arr_add_arr(doc, m);
      yyjson_mut_arr_add_float(doc, c,
                               glm::value_ptr(material.color_diffuse)[0]);
      yyjson_mut_arr_add_float(doc, c,
                               glm::value_ptr(material.color_diffuse)[1]);
      yyjson_mut_arr_add_float(doc, c,
                               glm::value_ptr(material.color_diffuse)[2]);
      yyjson_mut_arr_add_float(doc, c,
                               glm::value_ptr(material.color_diffuse)[3]);
    }

  } else if (const oDataPoint2 *variant =
                 std::get_if<oDataPoint2>(&node->variant)) {
    yyjson_mut_val *v = yyjson_mut_obj_add_arr(doc, node_json, "variant");
    yyjson_mut_arr_add_str(doc, v, "DATAPOINT");

    yyjson_mut_val *headers = yyjson_mut_arr_add_arr(doc, v);
    for (const auto &header : variant->headers) {
      yyjson_mut_val *header_json = yyjson_mut_arr_add_arr(doc, headers);
      yyjson_mut_arr_add_str(doc, header_json, header.header.c_str());
      yyjson_mut_arr_add_float(doc, header_json, header.min);
      yyjson_mut_arr_add_float(doc, header_json, header.max);
    }

  } else if (const Text3D *variant = std::get_if<Text3D>(&node->variant)) {
    yyjson_mut_val *v = yyjson_mut_obj_add_arr(doc, node_json, "variant");
    yyjson_mut_arr_add_str(doc, v, "TEXT3D");
    yyjson_mut_arr_add_str(doc, v, variant->string().c_str());
  }

  if (node->lua_params.size() > 0) {
    yyjson_mut_val *params =
        yyjson_mut_obj_add_arr(doc, node_json, "script-params");

    for (const auto &param : node->lua_params) {

      if (const Nil *value = std::get_if<Nil>(&param)) {
        yyjson_mut_arr_add_null(doc, params);

      } else if (const bool *value = std::get_if<bool>(&param)) {
        yyjson_mut_arr_add_bool(doc, params, *value);

      } else if (const int *value = std::get_if<int>(&param)) {
        yyjson_mut_arr_add_int(doc, params, *value);

      } else if (const float *value = std::get_if<float>(&param)) {
        yyjson_mut_arr_add_float(doc, params, *value);

      } else if (const std::string *value =
                     (std::string *)std::get_if<std::string>(&param)) {
        yyjson_mut_arr_add_str(doc, params, value->c_str());

      } else if (const Material *value = std::get_if<Material>(&param)) {
        yyjson_mut_val *p = yyjson_mut_arr_add_arr(doc, params);
        yyjson_mut_arr_add_str(doc, p, "MATERIAL");
        yyjson_mut_arr_add_str(doc, p, value->diffuse1.c_str());
        yyjson_mut_val *color = yyjson_mut_arr_add_arr(doc, p);
        yyjson_mut_arr_add_float(doc, color,
                                 glm::value_ptr(value->color_diffuse)[0]);
        yyjson_mut_arr_add_float(doc, color,
                                 glm::value_ptr(value->color_diffuse)[1]);
        yyjson_mut_arr_add_float(doc, color,
                                 glm::value_ptr(value->color_diffuse)[2]);
        yyjson_mut_arr_add_float(doc, color,
                                 glm::value_ptr(value->color_diffuse)[3]);

      } else if (const objH *value = std::get_if<objH>(&param)) {

        yyjson_mut_val *p = yyjson_mut_arr_add_arr(doc, params);
        yyjson_mut_arr_add_str(doc, p, "OBJH");
        if (objPool.get(*value)) {
          int16_t objidx = AddToPool2Json(pool2json, value->idx, counter);
          yyjson_mut_arr_add_int(doc, p, objidx);
        } else {
          yyjson_mut_arr_add_int(doc, p, -1);
        }

      } else if (const _ModelName *value =
                     (_ModelName *)std::get_if<_ModelName>(&param)) {
        yyjson_mut_val *p = yyjson_mut_arr_add_arr(doc, params);
        yyjson_mut_arr_add_str(doc, p, "MODEL");
        yyjson_mut_arr_add_str(doc, p, value->model.c_str());

      } else if (const _HeaderName *value =
                     (_HeaderName *)std::get_if<_HeaderName>(&param)) {

        yyjson_mut_val *p = yyjson_mut_arr_add_arr(doc, params);
        yyjson_mut_arr_add_str(doc, p, "HEADER");
        yyjson_mut_arr_add_str(doc, p, value->header.c_str());
      }
    }
  }

  if (node->children.size() > 0) {
    yyjson_mut_val *children =
        yyjson_mut_obj_add_arr(doc, node_json, "children");

    for (int16_t i = 0; i < node->children.size(); i++) {
      int16_t chidx = save_object_pool_recursive(doc, pool, counter, pool2json,
                                                 node->children[i]);
      yyjson_mut_arr_add_int(doc, children, chidx);
    }
  }

  return jdx;
}

void Scene::draw_models_recursive(int16_t idx, float delta,
                                  glm::mat4 parentTransform, objH parentObj) {

  Object *node = &objPool.get(objPool.rootH())[idx];
  glm::mat4 modelTransform = parentTransform * node->transform;
  parentObj = node->toHandle(idx) == selected_object || parentObj.idx == 0
                  ? node->toHandle(idx)
                  : parentObj;

  if (node->script_class_idx != -1) {
    const ScriptClass &sc =
        objPool.get_script_classes()[node->script_class_idx];

    lua_rawgeti(sc.L, LUA_REGISTRYINDEX, SCENE_SCRIPT_CLASSES_IDX);
    lua_rawgeti(sc.L, -1, node->script_class_idx + 1);
    lua_remove(sc.L, -2);

    if (sampler.emit_update) {
      if (lua_getfield(sc.L, -1, "on_update") == LUA_TFUNCTION) {
        Object **obj = (Object **)lua_newuserdata(sc.L, sizeof(Object *));
        *obj = node;
        luaL_getmetatable(sc.L, "ObjectRef");
        lua_setmetatable(sc.L, -2);
        Sampler **ud = (Sampler **)lua_newuserdata(sc.L, sizeof(void *));
        *ud = &sampler;
        luaL_getmetatable(sc.L, "Row");
        lua_setmetatable(sc.L, -2);
        call_lua(sc.L, 2);
      } else
        lua_remove(sc.L, -1);
    }

    if (lua_getfield(sc.L, -1, "on_frame") == LUA_TFUNCTION) {
      lua_remove(sc.L, -2);
      Object **obj = (Object **)lua_newuserdata(sc.L, sizeof(Object *));
      *obj = node;
      luaL_getmetatable(sc.L, "ObjectRef");
      lua_setmetatable(sc.L, -2);
      call_lua(sc.L, 1);
    } else {
      lua_remove(sc.L, -1);
      lua_remove(sc.L, -1);
    }
  }

  if (oModel *omodel = (oModel *)std::get_if<oModel>(&node->variant)) {
    if (!omodel->model.empty()) {
      ManagedModel *mmodel = resMan.GetModel(omodel->model);
      if (mmodel) {
        Renderer::Model &model = mmodel->model;

        if (omodel->animation_idx != -1 && model.GetAnimations().size() != 0 &&
            model.GetCurrentAnimation() - &model.GetAnimations()[0] !=
                omodel->animation_idx) {
          model.SetAnimation(&model.GetAnimations()[omodel->animation_idx]);
        } else if (omodel->animation_idx == -1) {
          model.SetAnimation(nullptr);
        }

        // if (animation_idx && *animation_idx != -1) {
        model.SetAnimationTime(omodel->animation_time);
        model.Advance(delta *
                      (omodel->animation_speed ? omodel->animation_speed : 1));
        omodel->animation_time = model.GetAnimationTime();
        // }

        Material *material = &omodel->materials[0];
        for (int i = 0; i < model.GetMaterials().size(); i++) {
          int diffuse1 = resMan.GetTexture(material->diffuse1);
          model.GetMaterials()[i] = Renderer::Material{
              .diffuse1 = diffuse1, .color_diffuse = material->color_diffuse};
          material++;
        }

        if (node->toHandle(idx) == selected_object) {
          glm::mat4 world2local = parentTransform * node->transform;
          ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                               mCurrentGizmoOperation, mCurrentGizmoMode,
                               glm::value_ptr(world2local), NULL, NULL,
                               glm::value_ptr(model.GetBoundingBox().min));

          node->transform = glm::inverse(parentTransform) * world2local;
          glUseProgram(selection_bb_program);
          glUniformMatrix4fv(selection_bb_model_loc, 1, GL_FALSE,
                             glm::value_ptr(world2local));
          glUniform3fv(selection_bb_min_loc, 1,
                       glm::value_ptr(model.GetBoundingBox().min));
          glUniform3fv(selection_bb_max_loc, 1,
                       glm::value_ptr(model.GetBoundingBox().max));
          render_selection_bb = true;

          modelTransform = parentTransform * node->transform;
        }

        model.transform = modelTransform;
        glm::vec3 color = glm::vec3(((float)parentObj.gen) / 256,
                                    ((float)(parentObj.idx % 256)) / 256,
                                    ((float)(parentObj.idx / 256)) / 256);

        glUseProgram(skinning_program);
        glUniform3fv(model_ID_loc, 1, glm::value_ptr(color));
        model.Draw();
      }
    }
  } else if (oDataPoint2 *odatapoint =
                 (oDataPoint2 *)std::get_if<oDataPoint2>(&node->variant)) {

    // if (sampler.sheet_idx != odatapoint->sheet_idx) {
    //   odatapoint->sheet_idx = sampler.sheet_idx;
    //   for (auto &hdr : odatapoint->headers) {
    //     auto header = sampler.headers.find(hdr.header);
    //     if (header != sampler.headers.end())
    //       hdr.header_idx = header->second;
    //     else
    //       hdr.header_idx = -1;
    //   }
    // }
    if (sampler.emit_update) {
      odatapoint->update_text(&sampler);
    }

    if (node->toHandle(idx) == selected_object) {
      glm::mat4 world2local = parentTransform * node->transform;
      ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                           ImGuizmo::TRANSLATE, mCurrentGizmoMode,
                           glm::value_ptr(world2local), NULL, NULL, NULL);

      node->transform = glm::inverse(parentTransform) * world2local;
      modelTransform = parentTransform * node->transform;
    }
  } else if (Text3D *otext3d = (Text3D *)std::get_if<Text3D>(&node->variant)) {
    if (node->toHandle(idx) == selected_object) {
      glm::mat4 world2local = parentTransform * node->transform;
      ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                           ImGuizmo::TRANSLATE, mCurrentGizmoMode,
                           glm::value_ptr(world2local), NULL, NULL, NULL);

      node->transform = glm::inverse(parentTransform) * world2local;
      modelTransform = parentTransform * node->transform;
    }
  }
  node->current_parent_transform = parentTransform;

  for (int16_t i = 0; i < node->children.size(); i++) {
    draw_models_recursive(node->children[i], delta, modelTransform, parentObj);
  }
}

void Scene::draw_onscreen_recursive(int16_t idx, float delta,
                                    glm::mat4 parentTransform, objH parentObj) {

  Object *node = &objPool.get(objPool.rootH())[idx];
  glm::mat4 modelTransform = parentTransform * node->transform;
  parentObj = node->toHandle(idx) == selected_object || parentObj.idx == 0
                  ? node->toHandle(idx)
                  : parentObj;

  if (oDataPoint2 *odatapoint =
          (oDataPoint2 *)std::get_if<oDataPoint2>(&node->variant)) {

    if (!odatapoint->display_text.string().empty()) {

      glm::vec3 color = glm::vec3(((float)parentObj.gen) / 256,
                                  ((float)(parentObj.idx % 256)) / 256,
                                  ((float)(parentObj.idx / 256)) / 256);

      glUseProgram(text_program);
      glUniform3fv(m_text_ID, 1, glm::value_ptr(color));
      odatapoint->display_text.draw(modelTransform);
    }
  }

  if (Text3D *otext3D = (Text3D *)std::get_if<Text3D>(&node->variant)) {

    if (!otext3D->string().empty()) {

      glm::vec3 color = glm::vec3(((float)parentObj.gen) / 256,
                                  ((float)(parentObj.idx % 256)) / 256,
                                  ((float)(parentObj.idx / 256)) / 256);

      glUseProgram(text_program);
      glUniform3fv(m_text_ID, 1, glm::value_ptr(color));
      otext3D->draw(modelTransform);
    }
  }

  for (int16_t i = 0; i < node->children.size(); i++) {
    draw_onscreen_recursive(node->children[i], delta, modelTransform,
                            parentObj);
  }
}

void Scene::Cleanup() {
  if (!tri_program)
    return;

  glDeleteVertexArrays(1, &tri_vao);
  glDeleteBuffers(1, &tri_fbo);
  glDeleteProgram(tri_program);
  tri_program = 0;

  if (!skybox_program)
    return;

  glDeleteVertexArrays(1, &skybox_vao);
  glDeleteBuffers(1, &skybox_fbo);
  glDeleteBuffers(1, &skybox_ebo);
  glDeleteProgram(skybox_program);
  skybox_program = 0;

  if (!skinning_program)
    return;

  glDeleteProgram(skinning_program);
  skinning_program = 0;

  if (!selection_bb_program)
    return;

  glDeleteVertexArrays(1, &selection_bb_vao);
  glDeleteBuffers(1, &selection_bb_fbo);
  glDeleteProgram(selection_bb_program);
  selection_bb_program = 0;

  if (!text_program)
    return;

  glDeleteProgram(text_program);
  glDeleteTextures(1, &m_text_texture);
  text_program = 0;
}

Scene::~Scene() {
  if (initialised)
    cleanup();
}
