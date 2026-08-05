#include <Renderer.hpp>
#include <glad/gl.h>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <Renderer_internal.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <placeholder_icon_img.h>
namespace Renderer {

void *Render::window_1st = nullptr;

bool Render::Init() {
  if (!glfwInit())
    return false;

  IMGUI_CHECKVERSION();
  return true;
}

void Render::Cleanup() { glfwTerminate(); }

void Render::PullEvents() { glfwPollEvents(); }

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
    throw std::runtime_error("Failed to create window");
  }

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

  if (!Render::window_1st)
    Render::window_1st = impl->window;

  impl->imctx = ImGui::CreateContext();
  ImGui::SetCurrentContext(impl->imctx);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(impl->window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
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

bool Render::WindowShouldClose() const { return glfwWindowShouldClose(impl->window); }

Result<rProgram, no_error> Render::LoadProgram(const char *name, const char *vs,
                                               const char *fs) {
  return impl->CreateProgram(name, vs, fs);
}

void Render::UnloadProgram(rProgram program) { glDeleteProgram(program); }

void Render::BeginFrame() {
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
