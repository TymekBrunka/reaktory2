#include <Errors/Errors.hpp>
#include <Renderer.hpp>
#include <Renderer_internal.hpp>
namespace Renderer {

Result<rShader, no_error> Render::Impl::createShader(GLenum shader_type,
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
  }
}

} // namespace Renderer
