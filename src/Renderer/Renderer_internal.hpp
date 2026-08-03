#pragma once
#include <Errors/Errors.hpp>
#include <Renderer.hpp>
#include <glad/gl.h>

#include <GLFW/glfw3.h>
namespace Renderer {

struct Render::Impl {
  rect_size window_size;
  GLFWwindow *window;

  Result<rShader, no_error> createShader(GLenum shader_type, const char *src);
};

} // namespace Renderer
