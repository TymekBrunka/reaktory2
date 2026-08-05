#include <Errors/Errors.hpp>
#include <Renderer.hpp>
#include <Renderer_internal.hpp>
#include <cstdio>
#include <cstring>
namespace Renderer {

Result<rShader, no_error> Render::Impl::CreateShader(GLenum shader_type,
                                                     const char *source) {

  if (shader_type != GL_VERTEX_SHADER && shader_type != GL_FRAGMENT_SHADER &&
      shader_type != GL_GEOMETRY_SHADER) {
    return Result<rShader, no_error>::ERR(false);
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

    char message[522] = {0};
    snprintf(message, 522, "%s linking error: ", shader_type_s);
    int offset = strlen(message);
    glGetShaderInfoLog(shader, 522 - offset, NULL, &message[offset]);
    fprintf(stderr, "%s\n", message);

    glDeleteShader(shader);
    return Result<rShader, no_error>::ERR(false);
  }

  return Result<rShader, no_error>::OK(shader);
}

Result<rProgram, no_error> Render::Impl::LinkProgram(rProgram program,
                                                     const char *name) {
  glLinkProgram(program);
  int linking_status;
  glGetProgramiv(program, GL_LINK_STATUS, &linking_status);

  if (!linking_status) {
    char message[590] = {0};
    snprintf(message, 590,
             "program '%s' linking error: ", name ? name : "(no name)");
    int offset = strlen(message);
    glGetProgramInfoLog(program, 590 - offset, NULL, &message[offset]);
    fprintf(stderr, "%s\n", message);

    glDeleteProgram(program);
    return Result<rProgram, no_error>::ERR(false);
  }

  return Result<rProgram, no_error>::OK(program);
}

bool Render::Impl::ValidateProgram(rProgram program, char *const message,
                                   int buflen) {
  glValidateProgram(program);
  int validation_status;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);

  if (!validation_status) {
    glGetProgramInfoLog(program, buflen, NULL, message);
    return false;
  }

  return true;
}

Result<rProgram, no_error> Render::Impl::CreateProgram(const char *name,
                                                       const char *vs_source,
                                                       const char *fs_source) {

  Result<rShader, no_error> vertex_shader_ =
      CreateShader(GL_VERTEX_SHADER, vs_source);

  if (!vertex_shader_.is_ok) {
    return Result<rProgram, no_error>::ERR(false);
  }

  Result<rShader, no_error> fragment_shader_ =
      CreateShader(GL_FRAGMENT_SHADER, fs_source);

  if (!fragment_shader_.is_ok) {
    return Result<rProgram, no_error>::ERR(false);
  }

  rProgram program = glCreateProgram();
  // labelObject(GL_PROGRAM, program, name);
  glAttachShader(program, vertex_shader_.ok_unchecked());
  glAttachShader(program, fragment_shader_.ok_unchecked());

  return LinkProgram(program, name);
}

} // namespace Renderer
