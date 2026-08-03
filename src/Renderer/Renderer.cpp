#include <Renderer.hpp>
#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <Renderer_internal.hpp>
namespace Renderer {

Render::Impl *Render::impl = nullptr;

bool Render::Init() {
  impl = new Render::Impl;

  if (!glfwInit())
    return false;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  impl->window_size = rect_size{640, 480};
  impl->window = glfwCreateWindow(640, 480, "Render", NULL, NULL);

  if (!impl->window) {
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(impl->window);
  glfwSwapInterval(1); // vsync: on
  gladLoadGL(glfwGetProcAddress);

  return true;
}

bool Render::WindowShouldClose() { return glfwWindowShouldClose(impl->window); }

void Render::BeginFrame() {
  glViewport(0, 0, impl->window_size.width, impl->window_size.height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::EndFrame() {
  glfwSwapBuffers(impl->window);
  glfwPollEvents();
}

void Render::Cleanup() {
  glfwDestroyWindow(impl->window);
  glfwTerminate();
  delete impl;
}

} // namespace Renderer
