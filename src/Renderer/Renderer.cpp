#include <Renderer.hpp>
#include <glad/gl.h>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <Renderer_internal.hpp>

#include "Translations.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <Errors/Errors.hpp>
#include <Logging.hpp>
#include <placeholder_icon_img.h>

#include "FontsAwesome/RobotoRegular.cpp"
#include "FontsAwesome/fa.h"
namespace Renderer {

void *Render::window_1st = nullptr;
double Render::timeout = 0.016; // 60 fps

bool Render::Init() {
  if (!glfwInit()) {
    Log::log_uform(Log::ERROR | Log::SEV_HIGH, 0, "RENDER",
                   Log::MSG_RENDER_INIT_ERROR);
    return false;
  }

  IMGUI_CHECKVERSION();
  Log::log_uform(Log::DEFAULT, 0, "RENDER", Log::MSG_RENDER_INIT_SUCCESS);
  return true;
}

void Render::Cleanup() { glfwTerminate(); }

void Render::PullEvents() { glfwWaitEventsTimeout(timeout); }

Render::Render(const char *title, rect_size window_size) {
  impl = new Render::Impl;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  impl->window_size = rect_size{640, 480};
  impl->window = glfwCreateWindow(
      window_size.width, window_size.height, title, NULL,
      Render::window_1st ? (GLFWwindow *)Render::window_1st : NULL);

  if (!impl->window) {
    delete[] impl;
    Log::log_uform(Log::ERROR | Log::SEV_HIGH, 0, "RENDER",
                   Log::MSG_RENDER_CREATE_WINDOW_FAILURE);
    throw std::runtime_error("Failed to create window");
  }

  Log::log_uform(Log::DEFAULT, 0, "RENDER",
                 Log::MSG_RENDER_CREATE_WINDOW_SUCCESS);

  Image icon{
      .width = placeholder_png_width,
      .height = placeholder_png_height,
      .channels = placeholder_png_channels,
      .pixels = (unsigned char *)placeholder_png_pixels,
  };

  SetWindowIcon(icon);

  glfwMakeContextCurrent(impl->window);
  glfwSwapInterval(1); // vsync: on
  gladLoadGL(glfwGetProcAddress);

  glfwSetWindowUserPointer(impl->window, this);
  glfwSetFramebufferSizeCallback(
      impl->window, [](GLFWwindow *window, int width, int height) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        render.impl->window_size = {width, height};
        glViewport(0, 0, width, height);

        if (render.impl->resize_callback)
          (*render.impl->resize_callback)(render, width, height);
      });
  glfwSetWindowIconifyCallback(
      impl->window, [](GLFWwindow *window, int iconified) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        render.impl->minimised = iconified;
        if (render.impl->minimise_callback)
          (*render.impl->minimise_callback)(render, iconified);
      });
  glfwSetWindowMaximizeCallback(
      impl->window, [](GLFWwindow *window, int maximised) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        if (render.impl->maximise_callback)
          (*render.impl->maximise_callback)(render, maximised);
      });
  glfwSetKeyCallback(impl->window, [](GLFWwindow *window, int key, int scancode,
                                      int action, int mods) {
    Render &render = *(Render *)glfwGetWindowUserPointer(window);
    if (render.impl->key_callback)
      (*render.impl->key_callback)(render, key, scancode, action, mods);
  });
  glfwSetCursorPosCallback(
      impl->window, [](GLFWwindow *window, double xpos, double ypos) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        if (render.impl->mouse_move_callback) {
          (*render.impl->mouse_move_callback)(render, xpos, ypos);
        }
      });
  glfwSetMouseButtonCallback(
      impl->window, [](GLFWwindow *window, int button, int action, int mods) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        if (render.impl->mouse_button_callback)
          (*render.impl->mouse_button_callback)(render, button, action, mods);
      });
  glfwSetScrollCallback(
      impl->window, [](GLFWwindow *window, double xoffset, double yoffset) {
        Render &render = *(Render *)glfwGetWindowUserPointer(window);
        if (render.impl->scroll_callback) {
          (*render.impl->scroll_callback)(render, xoffset, yoffset);
        }
      });
  glfwSetDropCallback(impl->window, [](GLFWwindow *window, int path_count,
                                       const char *paths[]) {
    Render &render = *(Render *)glfwGetWindowUserPointer(window);
    if (render.impl->drop_callback)
      (*render.impl->drop_callback)(render, path_count, paths);
  });
  Log::log_uform(Log::DEFAULT, 0, "RENDER",
                 Log::MSG_RENDER_SET_WINDOW_CALLBACKS);

  if (!Render::window_1st) {
    Render::window_1st = impl->window;
    Log::log_uform(Log::DEFAULT, 0, "RENDER",
                   Log::MSG_RENDER_SET_GLOBAL_GL_CTX);
  }

  impl->imctx = ImGui::CreateContext();
  ImGui::SetCurrentContext(impl->imctx);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::GetStyle().AntiAliasedLines = false;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(impl->window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  Log::log_uform(Log::DEFAULT, 0, "RENDER", Log::MSG_RENDER_CREATE_IMGUI_CTX);
}

Render::~Render() {
  if (impl) {
    ImGui::SetCurrentContext(impl->imctx);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(impl->imctx);
    glfwDestroyWindow(impl->window);
    delete[] impl;
  }
}

Render::Render(Render &&other) {
  impl = other.impl;
  other.impl = nullptr;
}

Render &Render::operator=(Render &&other) {
  if (this != &other) {
    impl = other.impl;
    other.impl = nullptr;
  }
  return *this;
}

void Render::makeContextCurrent() { glfwMakeContextCurrent(impl->window); }

void Render::SetWindowTitle(const char *title) {
  glfwSetWindowTitle(impl->window, title);
}

void Render::SetWindowIcon(const Image &icon_) {
  GLFWimage icon[1];
  icon[0].width = icon_.width;
  icon[0].height = icon_.height;
  icon[0].pixels = icon_.pixels;
  glfwSetWindowIcon(impl->window, 1, icon);
}

void Render::SetTargetFPS(double fps) { timeout = 1.0 / fps; }

GLFWwindow *Render::GetGLFWWindow() { return impl->window; }

bool Render::IsMinimised() const { return impl->minimised; }

void Render::SetResizeCallback(Render::RENDERframebuffersizefun *callback) {
  impl->resize_callback = callback;
}

void Render::SetMinimiseCallback(Render::RENDERwindowminimisefun *callback) {
  impl->minimise_callback = callback;
}

void Render::SetMaximiseCallback(Render::RENDERwindowmaximizefun *callback) {
  impl->maximise_callback = callback;
}

void Render::SetKeyCallback(Render::RENDERkeyfun *callback) {
  impl->key_callback = callback;
}

void Render::SetMouseMoveCallback(Render::RENDERcursorposfun *callback) {
  impl->mouse_move_callback = callback;
}

void Render::SetMouseButtonCallback(Render::RENDERmousebuttonfun *callback) {
  impl->mouse_button_callback = callback;
}

void Render::SetScrollCallback(Render::RENDERscrollfun *callback) {
  impl->scroll_callback = callback;
}

void Render::SetDropCallback(Render::RENDERdropfun *callback) {
  impl->drop_callback = callback;
}

rect_size Render::GetRenderSize() const { return impl->window_size; }

float Render::GetDelta() const { return impl->deltatime; }

bool Render::WindowShouldClose() const {
  return glfwWindowShouldClose(impl->window);
}

Result<rProgram, no_error> Render::LoadProgram(const char *name, const char *vs,
                                               const char *fs) {
  return impl->CreateProgram(name, vs, fs);
}

void Render::UnloadProgram(rProgram program) { glDeleteProgram(program); }

Result<rTexture2D, no_error> Render::LoadTexture(const Image &image,
                                                 bool pixelated, bool repeat) {

  if (image.channels <= 0 || image.channels > 4)
    return Result<rTexture2D, no_error>::ERR(false);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  GLenum interp_mode = pixelated ? GL_NEAREST : GL_LINEAR;
  GLenum clamp_mode = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;

  // paramethers MUST BE SET to load the texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp_mode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp_mode);

  if (image.mipmap_levels > 0) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, image.mipmap_levels);
  } else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp_mode);

  GLenum color_format = GL_RGBA;
  switch (image.channels) {
  case 1:
    color_format = GL_RED;
    break;
  case 2:
    color_format = GL_RG;
    break;
  case 3:
    color_format = GL_RGB;
    break;
  case 4:
    color_format = GL_RGBA;
    break;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, color_format, image.width, image.height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);

  if (image.mipmap_levels > 0)
    glGenerateMipmap(GL_TEXTURE_2D);

  if (glGetError() == GL_NO_ERROR)
    return Result<rTexture2D, no_error>::OK(texture);
  else
    return Result<rTexture2D, no_error>::ERR(false);
}

void Render::BindTexture(rTexture2D id, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, id);
}

void Render::UnloadTexture(rTexture2D id) { glDeleteTextures(1, &id); }

void Render::BeginFrame() {
  double current_frame_time = glfwGetTime();
  impl->deltatime = current_frame_time - impl->last_frame_time;
  impl->last_frame_time = current_frame_time;

  glViewport(0, 0, impl->window_size.width, impl->window_size.height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ImGui::SetCurrentContext(impl->imctx);
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Render::EndFrame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  // GLFWwindow *backup_current_context = glfwGetCurrentContext();
  // ImGui::UpdatePlatformWindows();
  // ImGui::RenderPlatformWindowsDefault();
  // glfwMakeContextCurrent(backup_current_context);
  glfwSwapBuffers(impl->window);
}

void Render::ClearBackground() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::ClearColor() { glClear(GL_COLOR_BUFFER_BIT); }

void Render::ClearDepth() { glClear(GL_DEPTH_BUFFER_BIT); }

} // namespace Renderer
