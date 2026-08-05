#pragma once
#include "imgui.h"
#include <Errors/Errors.hpp>
#include <Renderer.hpp>
#include <glad/gl.h>

#include <GLFW/glfw3.h>
namespace Renderer {

struct Render::Impl {
  float deltatime = 0;
  double last_frame_time = 0;
  rect_size window_size = {0, 0};
  GLFWwindow *window = nullptr;
  ImGuiContext *imctx = nullptr;

  Result<rShader, no_error> CreateShader(GLenum shader_type, const char *src);
  Result<rProgram, no_error> LinkProgram(rProgram program, const char *name);
  bool ValidateProgram(rProgram program, char *const message, int buflen);

  Result<rProgram, no_error>
  CreateProgram(const char *name, const char *vs_source, const char *fs_source);
};

} // namespace Renderer
