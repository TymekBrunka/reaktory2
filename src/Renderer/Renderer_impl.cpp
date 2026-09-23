#include "Logging.hpp"
#include "Translations.hpp"
#include "utility"
#include <Renderer.hpp>
#include <Renderer_internal.hpp>
#include <cstdio>
#include <cstring>
namespace Renderer {

std::expected<rShader, bool> Render::Impl::CreateShader(GLenum shader_type,
                                                        const char *source) {

  if (shader_type != GL_VERTEX_SHADER && shader_type != GL_FRAGMENT_SHADER &&
      shader_type != GL_GEOMETRY_SHADER) {
    return std::unexpected(false);
  }

  rShader shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int compilation_status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_status);

  if (!compilation_status) {
    const char *shader_type_s;

    switch (shader_type) {
    case GL_VERTEX_SHADER:
      shader_type_s = "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      shader_type_s = "fragment";
      break;
    case GL_GEOMETRY_SHADER:
      shader_type_s = "geometry";
      break;
    }

    char message[512] = {0};
    glGetShaderInfoLog(shader, 512, NULL, message);
    Log::log(Log::ERROR | Log::SEV_LOW, 0, "GL", TL(MSG_GL_ERROR_SHADER),
             std::make_format_args(shader_type_s, message));

    glDeleteShader(shader);
    return std::unexpected(false);
  }

  return (shader);
}

std::expected<rProgram, bool> Render::Impl::LinkProgram(rProgram program,
                                                        const char *name) {
  glLinkProgram(program);
  int linking_status;
  glGetProgramiv(program, GL_LINK_STATUS, &linking_status);

  if (!linking_status) {
    char message[512] = {0};
    glGetProgramInfoLog(program, 512, NULL, message);
    Log::log(Log::ERROR | Log::SEV_LOW, 0, "GL", TL(MSG_GL_ERROR_PROGRAM),
             std::make_format_args(name, message));

    glDeleteProgram(program);
    return std::unexpected(false);
  }

  return (program);
}

bool Render::Impl::ValidateProgram(rProgram program, char *const message,
                                   int buflen) {
  glValidateProgram(program);
  int validation_status;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);

  if (!validation_status) {
    glGetProgramInfoLog(program, buflen, NULL, message);
    Log::log(Log::ERROR | Log::SEV_LOW, 0, "GL", TL(MSG_GL_ERROR_VALIDATION),
             std::make_format_args(message));
    return false;
  }

  return true;
}

std::expected<rProgram, bool>
Render::Impl::CreateProgram(const char *name, const char *vs_source,
                            const char *fs_source) {

  auto vertex_shader_ = CreateShader(GL_VERTEX_SHADER, vs_source);

  if (!vertex_shader_.has_value()) {
    return std::unexpected(false);
  }

  auto fragment_shader_ = CreateShader(GL_FRAGMENT_SHADER, fs_source);

  if (!fragment_shader_.has_value()) {
    return std::unexpected(false);
  }

  rProgram program = glCreateProgram();
  // labelObject(GL_PROGRAM, program, name);
  glAttachShader(program, vertex_shader_.value());
  glAttachShader(program, fragment_shader_.value());

  return LinkProgram(program, name);
}

} // namespace Renderer
