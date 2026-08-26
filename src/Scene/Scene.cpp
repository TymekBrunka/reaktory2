#include <Errors/Errors.hpp>
#include <FileUtils.hpp>
#include <Logging.hpp>
#include <Renderer.hpp>
#include <Renderer_internal.hpp>
#include <Scene.hpp>
#include <iostream>

#include <shaders.h>
#include <staticassets_scene.h>
#include <stdatomic.h>

Renderer::rProgram Scene::skybox_program = 0;
Renderer::rProgram Scene::tri_program = 0;
Renderer::rFBO Scene::tri_fbo = 0;
Renderer::rVAO Scene::tri_vao = 0;

Scene::Scene(Scene &&other) {
  bool b = atomic_load_explicit(&other.can_rename, memory_order_seq_cst);
  atomic_store_explicit(&can_rename, b, memory_order_seq_cst);

  name = other.name;
  size = other.size;
  skybox_texture = other.skybox_texture;
  framebuffer = other.framebuffer;
  renderbuffer = other.renderbuffer;
  screen_canvas = other.screen_canvas;
  color_canvas = other.color_canvas;
  other.name = std::string();
  other.size = {640, 480};
  other.skybox_texture = 0;
  other.framebuffer = 0;
  other.renderbuffer = 0;
  other.screen_canvas = 0;
  other.color_canvas = 0;
}

Scene &Scene::operator=(Scene &&other) {
  if (this != &other) {
    bool b = atomic_load_explicit(&other.can_rename, memory_order_seq_cst);
    atomic_store_explicit(&can_rename, b, memory_order_seq_cst);

    name = other.name;
    size = other.size;
    skybox_texture = other.skybox_texture;
    framebuffer = other.framebuffer;
    renderbuffer = other.renderbuffer;
    screen_canvas = other.screen_canvas;
    color_canvas = other.color_canvas;
    other.name = std::string();
    other.size = {640, 480};
    other.skybox_texture = 0;
    other.framebuffer = 0;
    other.renderbuffer = 0;
    other.screen_canvas = 0;
    other.color_canvas = 0;
  }
  return *this;
}

struct Vertex {
  Renderer::vec2 pos;
  Renderer::vec3 col;
};

static const Vertex vertices[3] = {{{0.0f, 1.0f}, {1.f, 0.f, 0.f}},
                                   {{-1.0f, -1.0f}, {0.f, 1.f, 0.f}},
                                   {{1.0f, -1.0f}, {0.f, 0.f, 1.f}}};

static bool
write_file_if_not_exists_reported(const std::filesystem::path &filepath,
                                  const void *data, size_t size) {

  Errors::Result<Errors::no_error, int> err =
      FileUtils::WriteFileIfNotExists(filepath, data, size);
  if (!err.is_ok) {
    std::string path = filepath.string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             err.value.error == -1 ? TL(MSG_GENERIC_OPEN_ERROR)
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
  } catch (std::filesystem::filesystem_error &err) {
    std::string path1 = err.path1().string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             TL(MSG_SCENE_DIR_CREATE_ERROR), std::make_format_args(path1));
    return false;
  }

  if (!write_file_if_not_exists_reported(
          path / "document.json", document_json_data, document_json_size))
    return false;

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
  create_folder_structure(name, current_folder);

  Log::log(Log::DEFAULT, 0, "Scene", TL(MSG_SCENE_CREATE_SUCCESS),
           std::make_format_args(name));
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

  Log::log(Log::DEBUG, 0, "Scene",
           TL(MSG_SCENE_FRAMEBUFFER_CREATE_SUCCESS),
           std::make_format_args(screen_canvas, color_canvas));

  return true;
}

bool Scene::resize(Renderer::rect_size size) {
  if (this->size.width == size.width && this->size.height == size.height)
    return false;

  this->size = size;
  resize_framebuffer(size);

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

  Log::log(Log::VERBOSE, 0, "Scene",
           TL(MSG_SCENE_FRAMEBUFFER_RESIZE),
           std::make_format_args(screen_canvas, color_canvas, size.width,
                                 size.height));
}

bool Scene::Init(Renderer::Render &render) {
  Errors::Result<Renderer::rProgram, Errors::no_error> res_shader =
      render.LoadProgram("simple", (const char *)simple_vs_data,
                         (const char *)simple_fs_data);

  if (!res_shader.is_ok)
    return false;

  tri_program = res_shader.ok_unchecked();
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

  return true;
}

bool Scene::init(Renderer::Render &render) {
  if (!create_framebuffer())
    return false;

  return true;
}

void Scene::updateMousePos(const Renderer::rect_size &pos) { mpos = pos; }

void Scene::render() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

  glViewport(0, 0, size.width, size.height);
  glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(tri_program);
  glBindVertexArray(tri_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Scene::Cleanup() {
  if (!tri_program)
    return;

  glDeleteVertexArrays(1, &tri_vao);
  glDeleteBuffers(1, &tri_fbo);
  glDeleteProgram(tri_program);
}

Scene::~Scene() {}
