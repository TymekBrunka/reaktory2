#include "Model.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/geometric.hpp"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "glm/common.hpp"
#include "glm/trigonometric.hpp"
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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Renderer::rProgram Scene::skybox_program = 0;
Renderer::rFBO Scene::skybox_fbo = 0;
Renderer::rFBO Scene::skybox_ebo = 0;
Renderer::rVAO Scene::skybox_vao = 0;
Renderer::rProgram Scene::tri_program = 0;
Renderer::rFBO Scene::tri_fbo = 0;
Renderer::rVAO Scene::tri_vao = 0;

Renderer::rProgram Scene::skinning_program = 0;

Renderer::rLocation Scene::skybox_loc = 0;
Renderer::rLocation Scene::skybox_view_loc = 0;
Renderer::rLocation Scene::skybox_projection_loc = 0;

Renderer::Model *Scene::preview_model = nullptr;
Renderer::rLocation Scene::model_view_loc = 0;
Renderer::rLocation Scene::model_projection_loc = 0;

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

Scene::Scene(Scene &&other) {
  bool b = atomic_load_explicit(&other.can_rename, memory_order_seq_cst);
  atomic_store_explicit(&can_rename, b, memory_order_seq_cst);

  name = other.name;
  size = other.size;
  mpos = other.mpos;
  skybox_texture = other.skybox_texture;
  skybox_loc = other.skybox_loc;
  framebuffer = other.framebuffer;
  renderbuffer = other.renderbuffer;
  screen_canvas = other.screen_canvas;
  color_canvas = other.color_canvas;
  initialised = other.initialised;
  other.name = std::string();
  other.size = {640, 480};
  other.skybox_texture = 0;
  other.framebuffer = 0;
  other.renderbuffer = 0;
  other.screen_canvas = 0;
  other.color_canvas = 0;
  other.initialised = false;
}

Scene &Scene::operator=(Scene &&other) {
  if (this != &other) {
    bool b = atomic_load_explicit(&other.can_rename, memory_order_seq_cst);
    atomic_store_explicit(&can_rename, b, memory_order_seq_cst);

    name = other.name;
    size = other.size;
    mpos = other.mpos;
    skybox_texture = other.skybox_texture;
    skybox_loc = other.skybox_loc;
    framebuffer = other.framebuffer;
    renderbuffer = other.renderbuffer;
    screen_canvas = other.screen_canvas;
    color_canvas = other.color_canvas;
    initialised = other.initialised;
    other.name = std::string();
    other.size = {640, 480};
    other.skybox_texture = 0;
    other.framebuffer = 0;
    other.renderbuffer = 0;
    other.screen_canvas = 0;
    other.color_canvas = 0;
    other.initialised = false;
  }
  return *this;
}

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
  if (!std::filesystem::exists(current_folder)) {
    if (!create_folder_structure(name, current_folder)) {
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_SCENE_CREATE_ERROR), std::make_format_args(name));
      throw std::runtime_error("");
    }

    Log::log(Log::DEFAULT, 0, "Scene", TL(MSG_SCENE_CREATE_SUCCESS),
             std::make_format_args(name));
  } else {
    Log::log(Log::DEFAULT, 0, "Scene", TL(MSG_SCENE_OPEN_SUCCESS),
             std::make_format_args(name));
  }
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
  resize_framebuffer(size);

  projection =
      glm::perspective(glm::radians(90.0f),
                       (float)size.width / (float)size.height, 0.01f, 1000.0f);

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
  Errors::Result<Renderer::rProgram, Errors::no_error> res_shader =
      render.LoadProgram("simple", (const char *)simple_vs_data,
                         (const char *)simple_fs_data);

  if (!res_shader.is_ok)
    return false;

  tri_program = res_shader.ok_unchecked();

  res_shader = render.LoadProgram("skybox", (const char *)skybox_vs_data,
                                  (const char *)skybox_fs_data);

  if (!res_shader.is_ok) {
    render.UnloadProgram(tri_program);
    tri_program = 0;
    return false;
  }

  skybox_program = res_shader.ok_unchecked();

  res_shader = render.LoadProgram("skinning", (const char *)skinning_vs_data,
                                  (const char *)skinning_fs_data);

  if (!res_shader.is_ok) {
    render.UnloadProgram(tri_program);
    render.UnloadProgram(skybox_program);
    tri_program = 0;
    skybox_program = 0;
    return false;
  }

  skinning_program = res_shader.ok_unchecked();

  model_view_loc = glGetUniformLocation(skinning_program, "view");
  model_projection_loc = glGetUniformLocation(skinning_program, "projection");

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

  std::filesystem::path filepath =
      FileUtils::APP_ROOT / "scenes" / name / "skybox.png";
  Errors::Result<Renderer::Image, int> res_img_read =
      render.LoadImage(filepath.string().c_str());

  if (!res_img_read.is_ok) {
    std::string path = filepath.string();

    if (res_img_read.value.error == -1)
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_OPEN_ERROR), std::make_format_args(path));

    else if (res_img_read.value.error == 1)
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_GENERIC_READ_ERROR), std::make_format_args(path));

    else if (res_img_read.value.error == 2)
      Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
               TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));

    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialised = false;
    return false;
  }

  Errors::Result<Renderer::rTexture2D, Errors::no_error> res_txt =
      render.LoadTexture(res_img_read.ok_unchecked());
  if (!res_txt.is_ok) {
    std::string path = filepath.string();
    Log::log(Log::ERROR | Log::SEV_MED, 0, "Scene",
             TL(MSG_RENDER_LOAD_IMAGE_ERROR), std::make_format_args(path));

    free(res_img_read.ok_unchecked().pixels);

    glDeleteFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    initialised = false;
    return false;
  }

  free(res_img_read.ok_unchecked().pixels);

  skybox_texture = res_txt.ok_unchecked();
  skybox_loc = glGetUniformLocation(skybox_program, "skybox");
  glUniform1i(skybox_loc, 0);
  initialised = true;

  // ---------------------- model test
  preview_model = Renderer::Model::LoadFromFile(
      // "assets/example/models/RiggedSimple1.glb", true);
      "assets/example/models/CesiumMan.m3d", true);
  preview_model->SetAnimation(&preview_model->GetAnimations()[0]);
  return true;
}

void Scene::cleanup() {
  if (initialised) {
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

  preview_model->Advance(render.GetDelta());
  const glm::mat4 *transforms = preview_model->GetFinalMatrices();
  for (int i = 0; i < 100; i++) {
    snprintf(uniformNameBuffer, 100, "finalBonesMatrices[%d]", i);
    glUniformMatrix4fv(
        glGetUniformLocation(skinning_program, uniformNameBuffer), 1, GL_FALSE,
        glm::value_ptr(transforms[i]));
  }

  preview_model->Draw();

  // glUseProgram(tri_program);
  // glBindVertexArray(tri_vao);
  // glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Scene::Cleanup() {
  if (!tri_program)
    return;

  glDeleteVertexArrays(1, &tri_vao);
  glDeleteBuffers(1, &tri_fbo);
  glDeleteProgram(tri_program);

  if (!skybox_program)
    return;

  glDeleteVertexArrays(1, &skybox_vao);
  glDeleteBuffers(1, &skybox_fbo);
  glDeleteBuffers(1, &skybox_ebo);
  glDeleteProgram(skybox_program);

  if (!skinning_program)
    return;

  glDeleteProgram(skinning_program);
}

Scene::~Scene() {
  if (initialised)
    cleanup();
}
